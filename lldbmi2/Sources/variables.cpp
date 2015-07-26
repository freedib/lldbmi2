
#include "lldbmi2.h"
#include "log.h"
#include "variables.h"
#include "names.h"

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
	var = getVariable (frame, newexpression);
	logprintf (LOG_DEBUG, "getPeudoArrayVariable: expression %s -> %s\n", expression, newexpression);
	if (!var.IsValid() || var.GetError().Fail())
		return false;
	return true;
}

// from lldb-mi
SBValue
getVariable (SBFrame frame, const char *expression)
{
	logprintf (LOG_TRACE, "getVariable (0x%x, %s)\n", &frame, expression);
	SBValue var;
	if (strchr(expression,'@') != NULL)
		getPeudoArrayVariable (frame, expression, var);
	if (var.IsValid() && var.GetError().Success()) {
	}
	else if (expression[0] == '$')
		var = frame.FindRegister(expression);
#if 1
	else {
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
#else
	if (!var.IsValid() || var.GetError().Fail())
		var = frame.EvaluateExpression (expression);
#endif
	logprintf (LOG_DEBUG, "getVariable: expression=%s, name=%s, value=%s, changed=%d\n",
			expression, var.GetName(), var.GetValue(), var.GetValueDidChange());
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
			var.GetName(), var.GetNumChildren(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()), vartype.GetByteSize(),
			getNameForTypeClass(vartype.GetPointeeType().GetTypeClass()), getNameForBasicType(vartype.GetPointeeType().GetBasicType()), vartype.GetPointeeType().GetByteSize());
	logprintf (LOG_DEBUG, "updateVarState: Is(%-5s) = %d %d %d %d %s\n",
			var.GetName(), var.IsValid(), var.IsInScope(), var.IsDynamic(), var.IsSynthetic(), var.GetError().GetCString());
	SBStream stream;											// temp
	var.GetExpressionPath(stream);								// temp
	const char *varexpressionpath = stream.GetData();			// temp
	// Force a value to update
	var.GetValue();				// get value to activate changes
	int changes = var.GetValueDidChange();
	const char *summary = var.GetSummary();			// get value to activate changes  ????
	int varnumchildren = var.GetNumChildren();
	logprintf (LOG_DEBUG, "updateVarState: name=%s, varexpressionpath=%s, varnumchildren=%d, value=%s, summary=%s, changed=%d\n",
			var.GetName(), varexpressionpath, varnumchildren, var.GetValue(), summary, var.GetValueDidChange());
    // And update its children
	if (!vartype.IsPointerType() && !vartype.IsReferenceType() && !vartype.IsArrayType()) {
		for (int ichild = 0; ichild < varnumchildren; ++ichild) {
			SBValue child = var.GetChildAtIndex(ichild);
			if (child.IsValid() && var.GetError().Success() && depth>1) {
				child.SetPreferSyntheticValue (false);
				changes += updateVarState (child, depth-1);
			}
		}
	}
	return changes;
}


// search changed variables
char *
formatChangedList (char *changedesc, size_t descsize, SBValue var, bool &separatorvisible, int depth)
{
	logprintf (LOG_TRACE, "formatChangedList (%s, %d, 0x%x, %B, %d)\n", changedesc, descsize, &var, separatorvisible, depth);
	SBStream stream;
	var.GetExpressionPath(stream);
	const char *varexpressionpath = stream.GetData();
	if (varexpressionpath==NULL)
		return changedesc;
	logprintf (LOG_DEBUG, "formatChangedList: name=%s, expressionpath=%s, value=%s, summary=%s, changed=%d\n",
			var.GetName(), varexpressionpath, var.GetValue(), var.GetSummary(), var.GetValueDidChange());
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
	int childrenchanged = 0;
//	childrenchanged = updateVarState(var);
	if (var.GetValueDidChange() || childrenchanged>0) {
		const char *separator = separatorvisible? ",":"";
		const char *varinscope = var.IsInScope()? "true": "false";
		char vardesc[NAME_MAX];
		formatValue(vardesc,sizeof(vardesc),var,NO_SUMMARY);
		snprintf (changedesc, descsize,
			"%s{name=\"%s\",value=\"%s\",in_scope=\"%s\",type_changed=\"false\",has_more=\"0\"}",
			separator, varexpressionpath, vardesc, varinscope);
		separatorvisible = true;
		return changedesc;
	}
	if (!vartype.IsPointerType() && !vartype.IsReferenceType() && !vartype.IsArrayType()) {
		for (int ichild = 0; ichild < varnumchildren; ++ichild) {
			SBValue child = var.GetChildAtIndex(ichild);
			if (!child.IsValid() || var.GetError().Fail())
				continue;
			child.SetPreferSyntheticValue (false);
			// Handle composite types (i.e. struct or arrays)
			int changelength = strlen(changedesc);
            if (depth>1)
            	formatChangedList (changedesc+changelength, descsize-changelength, child, separatorvisible, depth-1);
		}
	}
	if (strlen(changedesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatChangedList: changedesc size (%d) too small\n", descsize);
	return changedesc;
}


// format a list of variables into a GDB string
// called for arguments and locals var after a breakpoint
char *
formatVariables (char *varsdesc, size_t descsize, SBValueList varslist)
{
	logprintf (LOG_TRACE, "formatVariables (%s, %d, 0x%x)\n", varsdesc, descsize, &varslist);
	*varsdesc = '\0';
	const char *separator="";
	for (size_t i=0; i<varslist.GetSize(); i++) {
		SBValue var = varslist.GetValueAtIndex(i);
		var.SetPreferSyntheticValue (false);
		if (var.IsValid() && var.GetError().Success()) {
			SBType vartype = var.GetType();
			logprintf (LOG_DEBUG, "formatVariables: var=%s, type class=%s, basic type=%s \n",
					var.GetName(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()));
			const char *varvalue = var.GetValue();
			// basic type valid only when type class is Builtin
			if ((vartype.GetBasicType()!=eBasicTypeInvalid && varvalue!=NULL) || true) {
				updateVarState (var);
				int varslength = strlen(varsdesc);
				char vardesc[NAME_MAX*2];
				formatValue (vardesc, sizeof(vardesc), var, FULL_SUMMARY);
				snprintf (varsdesc+varslength, descsize-varslength, "%s{name=\"%s\",value=\"%s\"}",
						separator, var.GetName(), vardesc);
				separator=",";
			}
			else
				logprintf (LOG_INFO, "formatVariables: var name=%s, invalid\n",	var.GetName());
		}
	}
	if (strlen(varsdesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatVariables: varsdesc size (%d) too small\n", descsize);
	return varsdesc;
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
formatSummary (char *summarydesc, size_t descsize, SBValue var)
{
	logprintf (LOG_TRACE, "formatSummary (%s, %d, 0x%x)\n", summarydesc, descsize, &var);
	//	value = "HFG\123klj\b"
	//	value={2,5,7,0 <repeat 5 times>, 7}
	//	value = {{a=1,b=0x33},...{a=1,b=2}}
	*summarydesc = '\0';
	const char *varsummary;
	SBType vartype = var.GetType();
	logprintf (LOG_DEBUG, "formatSummary: Var=%-5s: children=%-2d, typeclass=%-10s, basictype=%-10s, bytesize=%-2d, Pointee: typeclass=%-10s, basictype=%-10s, bytesize=%-2d\n",
				var.GetName(), var.GetNumChildren(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()), vartype.GetByteSize(),
				getNameForTypeClass(vartype.GetPointeeType().GetTypeClass()), getNameForBasicType(vartype.GetPointeeType().GetBasicType()), vartype.GetPointeeType().GetByteSize());
	if ((varsummary=var.GetSummary()) != NULL) {				// string
		snprintf (summarydesc, descsize, "%s", varsummary+1);	// copy and remove start apostrophe
		*(summarydesc+strlen(summarydesc)-1) = '\0';			// remove trailing apostrophe
		return summarydesc;
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
		char *ps=summarydesc;
		char vardesc[NAME_MAX];
		strlcat (ps++, "{", descsize--);
		for (int ichild=0; ichild<numchildren; ichild++) {
			SBValue child = var.GetChildAtIndex(ichild);
			if (!child.IsValid() || var.GetError().Fail())
				continue;
			child.SetPreferSyntheticValue (false);
			const char *childvalue = child.GetValue();
			if (childvalue != NULL)
				if (vartype.IsArrayType())
					snprintf (vardesc, sizeof(vardesc), "%s%s", separator, childvalue);
				else
					snprintf (vardesc, sizeof(vardesc), "%s%s=%s", separator, child.GetName(), childvalue);
			else {
				SBType childtype = var.GetType();
				lldb::addr_t childaddr;
				if (childtype.IsPointerType() || childtype.IsReferenceType())
					childaddr = var.GetValueAsUnsigned();
				else
					childaddr = var.GetLoadAddress();
				if (vartype.IsArrayType())
					snprintf (vardesc, sizeof(vardesc), "%s0x%llx", separator, childaddr);
				else
					snprintf (vardesc, sizeof(vardesc), "%s%s=0x%llx", separator, child.GetName(), childaddr);
			}
			if (descsize>0) {
				strlcat (ps, vardesc, descsize);
				ps += strlen (vardesc);
				descsize -= strlen (vardesc);
			}
			separator = ",";
		}
		if (descsize>0)
			strlcat (ps++, "}", descsize--);
		return summarydesc;
	}
	return NULL;
}

// format a variable description into a GDB string
char *
formatValue (char *vardesc, size_t descsize, SBValue var, VariableDetails details)
{
	logprintf (LOG_TRACE, "formatValue (%s, %d, 0x%x)\n", vardesc, descsize, &var);
	//	value = "HFG\123klj\b"
	//	value={2,5,7,0 <repeat 5 times>, 7}
	//	value = {{a=1,b=0x33},...{a=1,b=2}}
	SBType vartype = var.GetType();
	logprintf (LOG_DEBUG, "formatValue: Var=%-5s: children=%-2d, typeclass=%-10s, basictype=%-10s, bytesize=%-2d, Pointee: typeclass=%-10s, basictype=%-10s, bytesize=%-2d\n",
		var.GetName(), var.GetNumChildren(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()), vartype.GetByteSize(),
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
	const char *varname = var.GetName();
	char summarydesc[LINE_MAX];
	const char *varsummary = formatSummary (summarydesc, sizeof(summarydesc), var);
	const char *varvalue = var.GetValue();
	lldb::addr_t varaddr;
	if (vartype.IsPointerType() || vartype.IsReferenceType())
		varaddr = var.GetValueAsUnsigned();
	else
		varaddr = var.GetLoadAddress();
	logprintf (LOG_DEBUG, "formatValue: var=0x%llx, name=%s, summary=%s, value=%s, address=0x%llx\n",
			&var, varname, varsummary, varvalue, varaddr);

	*vardesc = '\0';
	if (varvalue != NULL) {			// basic types and arrays
		if (vartype.IsPointerType() || vartype.IsReferenceType() || vartype.IsArrayType()) {
			if (varsummary!=NULL && details==FULL_SUMMARY)
				snprintf (vardesc, descsize, "@1 %s \\\"%s\\\"", varvalue, varsummary);
			else
				snprintf (vardesc, descsize, "@2 %s {...}", varvalue);
		}
		else		// basic type
			strlcpy (vardesc, varvalue, descsize);
	}
	// classes and structures
	else if (varsummary!=NULL && details==FULL_SUMMARY)
		snprintf (vardesc, descsize, "@3 0x%llx \\\"%s\\\"", varaddr, varsummary);
	else
		snprintf (vardesc, descsize, "@4 0x%llx {...}", varaddr);
	if (strlen(vardesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatValue: vardesc size (%d) too small\n", descsize);
	return vardesc;
}



// get and correct and expression path
// correct cd->[] and cd.[] expressions to cd[]
// TODO: correct for argument like  struct CD (*cd) [2]. in hello.cpp reference works but not pointer
char *
formatExpressionPath (char *expressionpathdesc, size_t descsize, SBValue var)
{
	logprintf (LOG_TRACE, "formatExpressionPath (%s, %d, 0x%x)\n", expressionpathdesc, descsize, &var);
	// child expressions: var2.*b
	SBStream stream;
	var.GetExpressionPath (stream, true);
	strlcpy (expressionpathdesc, stream.GetData(), descsize);
	if (strlen(expressionpathdesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatValue: expressionpathdesc size (%d) too small\n", descsize);
	// correct expression
	char *pp;
	int suppress = 0;
	if ((pp=strstr(expressionpathdesc,"->[")) != NULL)
		suppress = 2;
	else if ((pp=strstr(expressionpathdesc,".[")) != NULL)
		suppress = 1;
	if (suppress > 0) {
		pp += suppress;		// remove the offending characters
		do
			*(pp-suppress) = *pp;
		while (*pp++);
	}
	return expressionpathdesc;
}

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
