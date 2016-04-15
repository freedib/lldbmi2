
#include "lldbmi2.h"
#include "log.h"
#include "variables.h"
#include "names.h"
#include <ctype.h>


extern LIMITS limits;


// TODO: check why walk/changed is slow
// TODO: error on evaluating...
// XMLTableImportContext::CreateChildContext() at XMLTableImport.cxx:495 0x46f759 ...
// mxTableModel	sdr::table::TableModelRef	Error: Multiple errors reported.\ Failed to execute MI command: -var-evaluate-expression this->mxRows._pInterface->mxTableModel \ Failed to execute MI command: -data-evaluate-expression this->mxRows._pInterface->mxTableModel \ Failed to execute MI command: -var-set-format this->mxRows._pInterface->mxTableModel octal \ Failed to execute MI command: -var-set-format this->mxRows._pInterface->mxTableModel decimal \ Failed to execute MI command: -var-set-format this->mxRows._pInterface->mxTableModel binary \ Failed to execute MI command: -var-set-format this->mxRows._pInterface->mxTableModel hexadecimal


// bug struct CD (*cdp)[2]
// gdb:     -var-create --thread 1 --frame 0 - * cdp		->    name="var4",numchild="1",type="CD (*)[2]"
//          -var-list-children var4                     	->    numchild="1" ... child={name="var4.*cdp",exp="*cdp",numchild="2",type="CD [2]"
//          -var-info-path-expression var4.*cdp				->    path_expr="*(cdp)"
//          -var-create --thread 1 --frame 0 - * &(*(cdp))	->    name="var7",numchild="1",value="0x7fff5fbff500",type="CD (*)[2]"
//          -var-create --thread 1 --frame 0 - * *(cdp)[0]	->    name="var8",numchild="1",value="{...}",type="CD"
//          -var-create --thread 1 --frame 0 - * *(cdp)[1]	->    name="var9",numchild="1",value="{...}",type="CD"
// lldbmi2: -var-create --thread 1 --frame 0 - * cdp		->    name="cdp"  ,numchild="2",type="CD (*)[2]"
//			-var-list-children cdp			 ->	numchild="2",children=[child={name="cdp[0]",exp="[0]",numchild="2",type="CD"},child={name="cdp[1]",exp="[1]",numchild="2",type="CD"}]"
//			-var-info-path-expression cdp[0] ->	path_expr="cdp[0]"
//			-var-info-path-expression cdp[1] ->	path_expr="cdp[1]"

// TODO: implement @ formats for structures like struct S. seems to be a bug in lldb
bool
getPeudoArrayVariable (SBFrame frame, const char *expression, SBValue &var)
{
	logprintf (LOG_TRACE, "getPeudoArrayVariable (0x%x, %s, 0x%x)\n", &frame, expression, &var);
// just support *((var)+0)@100 and &(*((var)+0)@100) forms
// char c[101];
// -var-create * *((c)+0)@100
// -var-create * *((c)+100)@1
	char *pe, e[NAME_MAX], *varname, *varoffset, *varlength, varpointername[NAME_MAX], vartype[NAME_MAX];
	strlcpy (e, expression, sizeof(e));
	pe = e;
	bool ampersand = false;
	if (*pe=='&' && *(pe+1)=='(') {				// check if ampersand
		ampersand = true;
		pe += 2;
	}
	if (*pe!='*' || *(pe+1)!='(' || *(pe+2)!='(')
		return false;
	varname = pe+3;								// get name
	if ((pe = strchr (varname,')')) == NULL)
		return false;
	*pe = '\0';
	if (*(pe+1) != '+' && !isdigit(*(pe+2)))
		return false;
	varoffset = pe+2;							// get offset
	if ((pe = strchr (varoffset,')')) == NULL)
		return false;
	*pe = '\0';
	if (*(pe+1) != '@' && !isdigit(*(pe+2)))
		return false;
	varlength = pe+2;							// get length
	if (ampersand) {
		if ((pe = strchr (varlength,')')) == NULL)
			return false;
		*pe = '\0';
	}
	snprintf (varpointername, sizeof(varpointername), "&%s", varname);	// get var type
	SBValue basevar = getVariable (frame, varpointername);				// vaname ¬ &var
	char newvartype[NAME_MAX];
	strlcpy (newvartype, basevar.GetTypeName(), sizeof(vartype));		// typename ~ char (*)[101]
	if ((pe = strchr (newvartype,'[')) == NULL)
		return false;
	*pe = '\0';
	char newexpression[NAME_MAX];				// create expression
	snprintf (newexpression, sizeof(newexpression), "%s*(%s[%s])&%s[%s]%s",	// (char(*)[100])&c[0] or &((char(*)[100])&c[0])
			ampersand?"&(":"", newvartype, varlength, varname, varoffset, ampersand?")":"");
	var = getVariable (frame, newexpression, false);
	logprintf (LOG_DEBUG, "getPeudoArrayVariable: expression %s -> %s\n", expression, newexpression);
	if (!var.IsValid() || var.GetError().Fail())
		return false;
	return true;
}

// get variable by trying standard approaches
// - from local variables
// - from variable path
// - from expression evaluation
bool
getStandardPathVariable (SBFrame frame, const char *expression, SBValue &var)
{
	logprintf (LOG_TRACE, "getStandardPathVariable (0x%x, %s, 0x%x)\n", &frame, expression, &var);
	if (true /*!var.IsValid() || var.GetError().Fail()*/) {
		const bool bArgs = true;
		const bool bLocals = true;
		const bool bStatics = true;
		const bool bInScopeOnly = false;
		const SBValueList varslist = frame.GetVariables (bArgs, bLocals, bStatics, bInScopeOnly);
		var = varslist.GetFirstValueByName (expression);
	}
	if (!var.IsValid() || var.GetError().Fail()) {
		var = frame.GetValueForVariablePath(expression);		// for c[0] or z.a
		if (!var.IsValid() || var.GetError().Fail())
			var = frame.EvaluateExpression (expression);
	}
	if (!var.IsValid() || var.GetError().Fail())
		return false;
	var.SetPreferSyntheticValue (false);
	return true;
}


// return the name of a variable
// adjust the name if NULL
const char *getName ( SBValue &var)
{
	const char *varname = var.GetName();
	if (varname == NULL)
		varname = "(anonymous)";
	return varname;
}


char *
strrstr (char *string, const char *find)
{
	size_t stringlen, findlen;
	char *cp;
	findlen = strlen(find);
	stringlen = strlen(string);
	if (findlen > stringlen)
		return NULL;
	for (cp = string + stringlen - findlen; cp >= string; cp--)
		if (strncmp(cp, find, findlen) == 0)
			return cp;
	return NULL;
}

// try go get a variable child from a path by walking its children
// while there are parts, search children for the remaining
bool
getDirectPathVariable (SBFrame frame, const char *expression, SBValue *foundvar, SBValue &parent, int depth)
{
	logprintf (LOG_TRACE, "getDirectPathVariable (0x%x, %s, 0x%x, 0x%x, %d)\n", &frame, expression, &foundvar, &parent, depth);
	char expression_parts[NAME_MAX], *pchildren=NULL;
	strlcpy (expression_parts, expression, sizeof(expression_parts));
	if (!parent.IsValid() || parent.GetError().Fail()) {
		// if root, search root variable. start from the bottom up to a valid variable
		do {
			if ((pchildren = strrchr (expression_parts, '.')) != NULL)
				*pchildren++ = '\0';
			else if ((pchildren = strrstr (expression_parts, "->")) != NULL) {
				*pchildren++ = '\0';
				*pchildren++ = '\0';
			}
			else if ((pchildren = strrchr (expression_parts, '[')) != NULL)
				*pchildren = '\0';
			else
				return false;					// no more parent/children form
		} while (!getStandardPathVariable (frame, expression_parts, parent));
		// copy again the string which may have now full of \0 ...
		strlcpy (expression_parts, expression, sizeof(expression_parts));
		// now follow the children up to the bottom
		return getDirectPathVariable (frame, pchildren, foundvar, parent, depth-1);
	}
	// split expression between parent and children
	if ((pchildren = strchr (expression_parts, '.')) != NULL)
		*pchildren++ = '\0';
	else if ((pchildren = strstr (expression_parts, "->")) != NULL) {
		*pchildren++ = '\0';
		*pchildren++ = '\0';
	}
	/*
	else if ((pchildren = strchr (expression_parts, '[')) != NULL) {
		size_t lep = strlen(expression_parts);		// shift children on the right
		if (lep<sizeof(expression_parts)-2) {		// room for shift and final \0
			char *pep = expression_parts+lep+1;
			do {
				*pep = *(pep-1);
			} while (--pep > pchildren);
			*pchildren++ = '\0';
		}
	}
	*/
	// search children var
	int parentnumchildren = parent.GetNumChildren();
	const char *parentname = getName (parent);
	logprintf (LOG_DEBUG, "getDirectPathVariable: expression part: parentname=%s child-searched=%s, other-children=%s\n",
			parentname, expression_parts, pchildren==NULL?"":pchildren);
	for (int ichild = 0; ichild < min(parentnumchildren,limits.children_max); ++ichild) {
		SBValue child = parent.GetChildAtIndex(ichild);
		if (child.IsValid() && child.GetError().Success() && depth>1) {
			child.SetPreferSyntheticValue (false);
			const char *childname = getName(child);
			logprintf (LOG_DEBUG, "getDirectPathVariable: scan children: parent=%s, expression_parts=%s, child=%s\n",
					parentname, expression_parts, childname);
			if (strcmp (expression_parts, childname) != 0)
				continue;
			if (pchildren == NULL) {
				*foundvar = child;
				return true;
			}
			// continue walking
			return getDirectPathVariable (frame, pchildren, foundvar, child, depth-1);
		}
	}
	return false;
}

// TODO: cast this to actual type
// example: &(this->maName.pData->buffer)
// becomes: ((rtl_uString *)((SdXMLDrawPageContext *) this)->maName.pData)->buffer
// try to cast &(var->...) &((VARTYPE)var)->...)
char *
castexpression (SBFrame frame, const char *expression, char *newexpression, size_t expressionsize)
{
	// try to cast the variable
	if (*expression!='&' || *(expression+1)!='(')
		return NULL;
	// isolate parent and children
	char subexpression[NAME_MAX];
	strlcpy (subexpression, expression, sizeof(subexpression));
	char *pe = subexpression+2;
	while (isalnum(*pe) || *pe=='_')
		++pe;
	if (*pe=='\0')
		return NULL;
	if (*pe!='-' || *(pe+1)!='>')
		return NULL;
	*pe++ = '\0';
	*pe++ = '\0';
	// search parent
	SBValue parent;
	getStandardPathVariable (frame, subexpression+2, parent);
	if (!parent.IsValid() || parent.GetError().Fail())
		return NULL;
	SBType parenttype = parent.GetType();
	const char *parenttypename = parenttype.GetDisplayTypeName();
	logprintf (LOG_DEBUG, "castexpression: parent=%s parenttypename=%s, parenttypedisplayname=%s\n",
				getName(parent), parenttypename, parenttype.GetDisplayTypeName());
	snprintf (newexpression, expressionsize, "(((%s)%s)->%s", parenttypename, subexpression+2, pe);	// a ) is left at the end
	return newexpression;
}

// get a variable by trying many approaches
SBValue
getVariable (SBFrame frame, const char *expression, bool tryDirect)
{
	logprintf (LOG_TRACE, "getVariable (0x%x, %s)\n", &frame, expression);
	SBValue var;
	if (strchr(expression,'@') != NULL)
		getPeudoArrayVariable (frame, expression, var);
	if ((!var.IsValid() || var.GetError().Fail()) && *expression=='$')
		var = frame.FindRegister(expression);
	if ((!var.IsValid() || var.GetError().Fail()) && tryDirect) {
		SBValue parent;
		getDirectPathVariable (frame, expression, &var, parent, limits.walk_depth_max);
	}
	if (!var.IsValid() || var.GetError().Fail())
		getStandardPathVariable (frame, expression, var);
	if ((!var.IsValid() || var.GetError().Fail()) && *expression=='&' && *(expression+1)=='(') {
		char newexpression[NAME_MAX];
		if (castexpression (frame, expression, newexpression, sizeof(newexpression)) != NULL)
			getStandardPathVariable (frame, newexpression, var);
	}
	logprintf (LOG_DEBUG, "getVariable: expression=%s, name=%s, value=%s, changed=%d\n",
			expression, getName(var), var.GetValue(), var.GetValueDidChange());
	if (var.IsValid() && var.GetError().Success())
		var.SetPreferSyntheticValue (false);
	return var;
}

// scan variables to reset their changed state
int
updateVarState (SBValue var, int depth)
{
	logprintf (LOG_TRACE, "updateVarState (0x%x, %d)\n", &var, depth);
	var.SetPreferSyntheticValue(false);
	SBType vartype = var.GetType();
	logprintf (LOG_DEBUG, "updateVarState: Var=%-5s: children=%-2d, typeclass=%-10s, basictype=%-10s, bytesize=%-2d, Pointee: typeclass=%-10s, basictype=%-10s, bytesize=%-2d\n",
			getName(var), var.GetNumChildren(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()), vartype.GetByteSize(),
			getNameForTypeClass(vartype.GetPointeeType().GetTypeClass()), getNameForBasicType(vartype.GetPointeeType().GetBasicType()), vartype.GetPointeeType().GetByteSize());
	logprintf (LOG_NONE, "updateVarState: Is(%-5s) = %d %d %d %d %s\n",
			getName(var), var.IsValid(), var.IsInScope(), var.IsDynamic(), var.IsSynthetic(), var.GetError().GetCString());
	static StringB expressionpathdescB(NAME_MAX);									// temp
	expressionpathdescB.clear();													// temp
	char *expressionpathdesc = formatExpressionPath (expressionpathdescB, var);		// temp
	// Force a value to update
	var.GetValue();									// get value to activate changes
	int changes = var.GetValueDidChange();
	const char *summary = var.GetSummary();			// get value to activate changes  ????
	int varnumchildren = var.GetNumChildren();
	logprintf (LOG_DEBUG, "updateVarState: name=%s, varexpressionpath=%s, varnumchildren=%d, value=%s, summary=%s, changed=%d\n",
			getName(var), expressionpathdesc, varnumchildren, var.GetValue(), summary, var.GetValueDidChange());
    // And update its children
	if (/*!vartype.IsPointerType() && !vartype.IsReferenceType() &&*/ !vartype.IsArrayType()) {
		for (int ichild = 0; ichild < min(varnumchildren,limits.children_max); ++ichild) {
			SBValue child = var.GetChildAtIndex(ichild);
			if (child.IsValid() && var.GetError().Success() && depth>1) {
				child.SetPreferSyntheticValue (false);
				changes += updateVarState (child, depth-1);
			}
		}
	}
	return changes;
}



// get and correct and expression path
// correct cd->[] and cd.[] expressions to cd[]
// TODO: correct for argument like  struct CD (*cd) [2]. in tests.cpp reference works but not pointer
char *
formatExpressionPath (SBValue var)
{
	static StringB expressionpathdescB(NAME_MAX);
	expressionpathdescB.clear();
	return formatExpressionPath (expressionpathdescB, var);
}

char *
formatExpressionPath (StringB &expressionpathdescB, SBValue var)
{
	logprintf (LOG_TRACE, "formatExpressionPath (0x%x, 0x%x)\n", &expressionpathdescB, &var);
	// child expressions: var2.*b
	SBStream stream;
	var.GetExpressionPath (stream, true);
	expressionpathdescB.append(stream.GetData());
	const char *varname = var.GetName();
	if (varname==NULL)
		expressionpathdescB.append("(anonymous)");
	logprintf (LOG_DEBUG, "formatExpressionPath: expressionpathdesc=%s\n",
			varname, expressionpathdescB.c_str());
	// correct expression
	char *pstr=expressionpathdescB.c_str(), *pp;
	int suppress;
	do {
		suppress = 0;
		if ((pp=strstr(pstr,"->[")) != NULL)
			suppress = 2;
		else if ((pp=strstr(pstr,".[")) != NULL)
			suppress = 1;
		else if ((pp=strstr(pstr,"..")) != NULL)
			suppress = 1;		// may happen if unions with no name
		if (suppress > 0)		// remove the offending characters
			expressionpathdescB.clear(suppress,pp-pstr);		// bytes, start
	} while (suppress>0);
	if (expressionpathdescB.size()>0)
		if (expressionpathdescB.c_str()[expressionpathdescB.size()-1]=='.')
			expressionpathdescB.c_str()[expressionpathdescB.size()-1] = '\0';		// remove final '.'
	return expressionpathdescB.c_str();
}

// list children variables
char *
formatChildrenList (SBValue var, char *expression, int threadindexid, int &varnumchildren)
{
	static StringB childrendescB(BIG_LINE_MAX);
	childrendescB.clear();
	return formatChildrenList (childrendescB, var, expression, threadindexid, varnumchildren);
}

char *
formatChildrenList (StringB &childrendescB, SBValue var, char *expression, int threadindexid, int &varnumchildren)
{
	logprintf (LOG_TRACE, "formatChildrenList (0x%x, 0x%x, %s, %d, %d)\n", &childrendescB, &var, expression, threadindexid, varnumchildren);
	varnumchildren = var.GetNumChildren();
	const char *sep="";
	int ichild;
	for (ichild=0; ichild<min(varnumchildren,limits.children_max); ichild++) {
		SBValue child = var.GetChildAtIndex(ichild);
		if (!child.IsValid())
			continue;
		const char *childname = getName(child);				// displayed name
		if (child.GetError().Fail())
			logprintf (LOG_DEBUG, "formatChildrenList: error on child %s: %s\n",
					childname==NULL? "": childname, child.GetError().GetCString());
		child.SetPreferSyntheticValue (false);
		int childnumchildren = child.GetNumChildren();
		SBType childtype = child.GetType();
		const char *displaytypename = childtype.GetDisplayTypeName();
		static StringB expressionpathdescB(NAME_MAX);			// real path
		expressionpathdescB.clear();							// clear previous buffer content
		if (strcmp(childname,displaytypename)==0)				// if extends class name
			expressionpathdescB.catsprintf("%s.%s", expression, childname);
		else {
			formatExpressionPath (expressionpathdescB, child);
			if (strcmp((const char *)expression,(const char *)expressionpathdescB.c_str())==0) {
				SBType vartype = var.GetType();
				logprintf (LOG_ALL, "formatChildrenList: Var=%-5s: children=%-2d, typeclass=%-10s, basictype=%-10s, bytesize=%-2d, Pointee: typeclass=%-10s, basictype=%-10s, bytesize=%-2d\n",
						getName(var), var.GetNumChildren(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()), vartype.GetByteSize(),
						getNameForTypeClass(vartype.GetPointeeType().GetTypeClass()), getNameForBasicType(vartype.GetPointeeType().GetBasicType()), vartype.GetPointeeType().GetByteSize());
				if (childnumchildren>0)
					// special case with casts like StringB or Vector. Add the name to the expression
					childrendescB.catsprintf (".%s", childname);
			}
		}
		logprintf (LOG_DEBUG, "formatChildrenList (expressionpathdesc=%s, childchildren=%d, childname=%s)\n",
				expressionpathdescB.c_str(), childnumchildren, childname);
		// [child={name="var2.*b",exp="*b",numchild="0",type="char",thread-id="1"}]
		childrendescB.catsprintf ("%schild={name=\"%s\",exp=\"%s\",numchild=\"%d\","
			"type=\"%s\",thread-id=\"%d\"}",
			sep,expressionpathdescB.c_str(),childname,childnumchildren,displaytypename,threadindexid);
		sep = ",";
	}
	return childrendescB.c_str();
}

// search changed variables
char *
formatChangedList (SBValue var, bool &separatorvisible, int depth)
{
	static StringB changedescB(BIG_LINE_MAX);
	changedescB.clear();
	return formatChangedList (changedescB, var, separatorvisible, depth);
}

char *
formatChangedList (StringB &changedescB, SBValue var, bool &separatorvisible, int depth)
{
	logprintf (LOG_TRACE, "formatChangedList (0x%x, 0x%x, %B, %d)\n", &changedescB, &var, separatorvisible, depth);
	static StringB expressionpathdescB(NAME_MAX);
	expressionpathdescB.clear();
	formatExpressionPath (expressionpathdescB, var);
	if (expressionpathdescB.size()==0)
		return changedescB.c_str();
	logprintf (LOG_DEBUG, "formatChangedList: name=%s, expressionpath=%s, value=%s, summary=%s, changed=%d\n",
			getName(var), expressionpathdescB.c_str(), var.GetValue(), var.GetSummary(), var.GetValueDidChange());
	var.GetValue();					// required to get value to activate changes
	var.GetSummary();				// required to get value to activate changes
	SBType vartype = var.GetType();
	int varnumchildren = var.GetNumChildren();
	if (vartype.IsReferenceType() && varnumchildren==1) {
		// use child if reference. class (*) [] format
		logprintf (LOG_DEBUG, "formatValue: special case class (*) []\n");
		SBValue child = var.GetChildAtIndex(0);
		if (child.IsValid() && var.GetError().Success()) {
			child.SetPreferSyntheticValue (false);
			var = child;
			vartype = var.GetType();
			varnumchildren = var.GetNumChildren();
		}
	}
//	int varandchildrenchanged = updateVarState(var, limits.change_depth_max);
	int varchanged = var.GetValueDidChange();
	if (varchanged) {
		const char *separator = separatorvisible? ",":"";
		const char *varinscope = var.IsInScope()? "true": "false";
		static StringB vardescB(VALUE_MAX);
		vardescB.clear();						// clear previous buffer content
		formatValue(vardescB,var,NO_SUMMARY);
		changedescB.catsprintf ("%s{name=\"%s\",value=\"%s\",in_scope=\"%s\",type_changed=\"false\",has_more=\"0\"}",
			separator, expressionpathdescB.c_str(), vardescB.c_str(), varinscope);
		separatorvisible = true;
	//	return changedesc;
	}
	if (/*varandchildrenchanged>varchanged && */
			/*!vartype.IsPointerType() && !vartype.IsReferenceType() && */ !vartype.IsArrayType()) {
		for (int ichild = 0; ichild < min(varnumchildren,limits.children_max); ++ichild) {
			SBValue child = var.GetChildAtIndex(ichild);
			if (!child.IsValid() || var.GetError().Fail())
				continue;
			child.SetPreferSyntheticValue (false);
			// Handle composite types (i.e. struct or arrays)
            if (depth>1)
            	formatChangedList (changedescB, child, separatorvisible, depth-1);
		}
	}
	return changedescB.c_str();
}


// format a list of variables into a GDB string
// called for arguments and locals var after a breakpoint
char *
formatVariables (SBValueList varslist)
{
	static StringB varsdescB(BIG_LINE_MAX);
	varsdescB.clear();
	return formatVariables (varsdescB, varslist);
}

char *
formatVariables (StringB &varsdescB, SBValueList varslist)
{
	logprintf (LOG_TRACE, "formatVariables (0x%x, 0x%x)\n", varsdescB.c_str(), &varslist);
	varsdescB.clear();
	const char *separator="";
	for (size_t i=0; i<varslist.GetSize(); i++) {
		SBValue var = varslist.GetValueAtIndex(i);
		var.SetPreferSyntheticValue (false);
		if (var.IsValid() && var.GetError().Success()) {
			SBType vartype = var.GetType();
			logprintf (LOG_DEBUG, "formatVariables: var=%s, type class=%s, basic type=%s \n",
					getName(var), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()));
			const char *varvalue = var.GetValue();
			// basic type valid only when type class is Builtin
			if ((vartype.GetBasicType()!=eBasicTypeInvalid && varvalue!=NULL) || true) {
			//	updateVarState (var, limits.change_depth_max);
				static StringB vardescB(BIG_VALUE_MAX);
				vardescB.clear();								// clear previous buffer content
				formatValue (vardescB, var, FULL_SUMMARY);
				varsdescB.catsprintf ("%s{name=\"%s\",value=\"%s\"}",	separator, getName(var), vardescB.c_str());
				separator=",";
			}
			else
				logprintf (LOG_INFO, "formatVariables: var name=%s, invalid\n",	getName(var));
		}
	}
	return varsdescB.c_str();
}

/*
	typedef struct CD { int c; const char *d;} CDCD;
	class AB {public: int a; int b; int c;};

                                   var                        Pointee                    Dereferenced
                       children typeclass    bytesize    type            bytesize        type      bytesize
	char s[7]              7      Array        7         Invalid                        Array          7
	struct CD cd[3]        3      Array       48         Invalid                        Array         48
	double d[5]            5      Array       40         Invalid                        Array         40

	int *i                 1      Pointer      8         Builtin:Int          4         Pointer        8
	char *s                1      Pointer      8         Builtin:SignedChar   1         Pointer        8
	const char*v           1      Pointer      8         Builtin:SignedChar   1         Pointer        8
	double *d              1      Pointer      8         Builtin:Double       8         Pointer        8

	struct CD (*cdp) [3]   3      Pointer      8         Array               48         Pointer        8

	char (&sr)[7]          7      Reference    8         Array                7         Array          7
	struct CD (&cdr)[3]    3      Reference    8         Array               48         Array         48
	bool &b                1      Reference    8         Builtin:Bool         1         Builtin:Bool   1
	AB   &ab               3      Reference    8         Class               12         Class         12
*/
char *
formatSummary (SBValue var)
{
	static StringB summarydescB(BIG_LINE_MAX);
	summarydescB.clear();
	return formatSummary (summarydescB, var);
}

char *
formatSummary (StringB &summarydescB, SBValue var)
{
	logprintf (LOG_TRACE, "formatSummary (0x%x, 0x%x)\n", &summarydescB, &var);
	//	value = "HFG\123klj\b"
	//	value={2,5,7,0 <repeat 5 times>, 7}
	//	value = {{a=1,b=0x33},...{a=1,b=2}}
	const char *varsummary;
	SBType vartype = var.GetType();
	logprintf (LOG_DEBUG, "formatSummary: Var=%-5s: children=%-2d, typeclass=%-10s, basictype=%-10s, bytesize=%-2d, Pointee: typeclass=%-10s, basictype=%-10s, bytesize=%-2d\n",
				getName(var), var.GetNumChildren(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()), vartype.GetByteSize(),
				getNameForTypeClass(vartype.GetPointeeType().GetTypeClass()), getNameForBasicType(vartype.GetPointeeType().GetBasicType()), vartype.GetPointeeType().GetByteSize());
	if ((varsummary=var.GetSummary()) != NULL) {				// string
		summarydescB.append(varsummary+1);						// copy and remove start apostrophe
		summarydescB.clear(1,summarydescB.size()-1);			// remove trailing apostrophe
		return summarydescB.c_str();
	}
	int datasize=0;
	TypeClass vartypeclass = vartype.GetTypeClass();
	SBType pointeetype = vartype.GetPointeeType();
	int numchildren = var.GetNumChildren();
	if (vartype.IsArrayType())
		datasize = var.GetByteSize();
	else if (vartype.IsReferenceType())
		datasize = pointeetype.GetByteSize();
	else if (vartype.IsPointerType()) {
		datasize = pointeetype.GetByteSize();
	}

	if (vartypeclass==eTypeClassClass || vartypeclass==eTypeClassStruct || vartypeclass==eTypeClassUnion || vartype.IsArrayType()) {
		const char *separator="";
		static StringB vardescB(VALUE_MAX);
		summarydescB.append("{");
		for (int ichild=0; ichild<min(numchildren,limits.children_max); ichild++) {
			SBValue child = var.GetChildAtIndex(ichild);
			if (!child.IsValid() || var.GetError().Fail())
				continue;
			child.SetPreferSyntheticValue (false);
			const char *childvalue = child.GetValue();
			vardescB.clear();						// clear previous buffer content
			if (childvalue != NULL)
				if (vartype.IsArrayType())
					vardescB.catsprintf ("%s%s", separator, childvalue);
				else
					vardescB.catsprintf ("%s%s=%s", separator, getName(child), childvalue);
			else {
				SBType childtype = var.GetType();
				lldb::addr_t childaddr;
				if (childtype.IsPointerType() || childtype.IsReferenceType())
					childaddr = var.GetValueAsUnsigned();
				else
					childaddr = var.GetLoadAddress();
				if (vartype.IsArrayType())
					vardescB.catsprintf ("%s0x%llx", separator, childaddr);
				else
					vardescB.catsprintf ("%s%s=0x%llx", separator, getName(child), childaddr);
			}
			summarydescB.append(vardescB.c_str());
			separator = ",";
		}
		summarydescB.append("}");
		return summarydescB.c_str();
	}
	return NULL;
}

// format a variable description into a GDB string
char *
formatValue (SBValue var, VariableDetails details)
{
	static StringB vardescB(VALUE_MAX);
	vardescB.clear();
	return formatValue (vardescB, var, details);
}

char *
formatValue (StringB &vardescB, SBValue var, VariableDetails details)
{
	logprintf (LOG_TRACE, "formatValue (0x%x, 0x%x, %x)\n", &vardescB, &var, details);
	//	value = "HFG\123klj\b"
	//	value={2,5,7,0 <repeat 5 times>, 7}
	//	value = {{a=1,b=0x33},...{a=1,b=2}}
	vardescB.clear();
	if (var.GetError().Fail())		// invalid value. show nothing
		return vardescB.c_str();
	SBType vartype = var.GetType();
	logprintf (LOG_DEBUG, "formatValue: Var=%-5s: children=%-2d, typeclass=%-10s, basictype=%-10s, bytesize=%-2d, Pointee: typeclass=%-10s, basictype=%-10s, bytesize=%-2d\n",
		getName(var), var.GetNumChildren(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()), vartype.GetByteSize(),
		getNameForTypeClass(vartype.GetPointeeType().GetTypeClass()), getNameForBasicType(vartype.GetPointeeType().GetBasicType()), vartype.GetPointeeType().GetByteSize());
	int varnumchildren = var.GetNumChildren();
	if (vartype.IsReferenceType() && varnumchildren==1) {
		// use child if reference. class (*) [] format
		logprintf (LOG_DEBUG, "formatValue: special case class (*) []\n");
		SBValue child = var.GetChildAtIndex(0);
		if (child.IsValid() && var.GetError().Success()) {
			child.SetPreferSyntheticValue (false);
			var = child;
			vartype = var.GetType();
			varnumchildren = var.GetNumChildren();
		}
	}
	const char *varname = getName(var);
	static StringB summarydescB(BIG_LINE_MAX);
	summarydescB.clear();								// clear previous buffer content
	formatSummary (summarydescB, var);
	const char *varvalue = var.GetValue();
	lldb::addr_t varaddr;
	if (vartype.IsPointerType() || vartype.IsReferenceType())
		varaddr = var.GetValueAsUnsigned();
	else
		varaddr = var.GetLoadAddress();
	logprintf (LOG_DEBUG, "formatValue: var=0x%llx, name=%s, summary=%s, value=%s, address=0x%llx\n",
			&var, varname, summarydescB.c_str(), varvalue, varaddr);

	if (varvalue != NULL) {			// basic types and arrays
		if (vartype.IsPointerType() || vartype.IsReferenceType() || vartype.IsArrayType()) {
			if (summarydescB.size()>0 && details==FULL_SUMMARY)
				vardescB.catsprintf ("%s \\\"%s\\\"", varvalue, summarydescB.c_str());
			else
				vardescB.catsprintf ("%s {...}", varvalue);
		}
		else		// basic type
			vardescB.append(varvalue);
	}
	// classes and structures
	else if (summarydescB.size()>0 && details==FULL_SUMMARY)
		vardescB.catsprintf ("0x%llx \\\"%s\\\"", varaddr, summarydescB.c_str());
	else
		vardescB.catsprintf ("0x%llx {...}", varaddr);
	return vardescB.c_str();
}
