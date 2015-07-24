
#include "lldbmi2.h"
#include "log.h"
#include "variables.h"
#include "names.h"


bool enable_arrays_change_detection = false;	// set true to enable arrays change detection. still in beta

extern bool global_istest;						// true when debugging lldbmi2 with lldbmi2

// should be in a class to be allocated dynamically
static TRACKED_VAR trackedvars[TRACKED_VARS_MAX];
size_t numtrackedvars;


// search a var in tracked vars list
int
getTrackedVar (addr_t functionaddress, SBValue var)
{
	const char *varname = var.GetName();
	for (size_t tvi=0; tvi<numtrackedvars; tvi++) {
		logprintf (LOG_NONE, "getTrackedVar: test var %llx:%s against %llx:%s\n",
				functionaddress, varname,
				trackedvars[tvi].functionaddress, trackedvars[tvi].varname);
		if (functionaddress==trackedvars[tvi].functionaddress && strcmp(varname,trackedvars[tvi].varname)==0) {
			logprintf (LOG_NONE, "getTrackedVar: got var at index %d\n", tvi);
			return tvi;
		}
	}
	return EOF;
}


// get real data size
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
void
getDataInfo (SBValue var, int *dataitems, int *datasize)
{
	*dataitems = *datasize = 0;
	SBType vartype = var.GetType();
	SBType pointeetype = vartype.GetPointeeType();
	logprintf (LOG_NONE, "getDataInfo: Var=%s, num children=%d, type class=%s, basic type=%s, byte size=%d\n",
				var.GetName(), var.GetNumChildren(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()), vartype.GetByteSize());
	logprintf (LOG_NONE, "getDataInfo: Pointee: type class=%s, basic type=%s, byte size=%d\n",
				getNameForTypeClass(vartype.GetPointeeType().GetTypeClass()), getNameForBasicType(vartype.GetPointeeType().GetBasicType()), vartype.GetPointeeType().GetByteSize());
	*dataitems = var.GetNumChildren();
	if (vartype.IsArrayType())
		*datasize = var.GetByteSize();
	else if (vartype.IsReferenceType())
		*datasize = pointeetype.GetByteSize();
	else if (vartype.IsPointerType()) {
		*datasize = pointeetype.GetByteSize();
		BasicType basictype = pointeetype.GetBasicType();
		if (basictype==eBasicTypeChar || basictype==eBasicTypeSignedChar || basictype==eBasicTypeUnsignedChar) {
			*dataitems = ARRAY_MAX;		// take a chance on character strings
			*datasize = ARRAY_MAX;		// take a chance on character strings
		}
	}
}

// add a var in tracked vars list
int
addTrackedVar (addr_t functionaddress, SBValue var, bool checkifexists)
{
	if (!enable_arrays_change_detection && !global_istest)
		return 0;
	const char *varname = var.GetName();
	SBType vartype = var.GetType();
	int tvi;
	if (checkifexists)
		if ((tvi=getTrackedVar(functionaddress,var)) >= 0)
			return tvi;									// already exists
	if (numtrackedvars < TRACKED_VARS_MAX) {
		int dataitems, datasize;
		getDataInfo (var, &dataitems, &datasize);
		if (datasize <= 0)
			return EOF;
		dataitems = min (dataitems, ARRAY_MAX-1);
		datasize = min (datasize, ARRAY_MAX-1);
		trackedvars[numtrackedvars].functionaddress = functionaddress;
		strlcpy (trackedvars[numtrackedvars].varname, varname, sizeof(trackedvars[numtrackedvars].varname));
		if (vartype.IsPointerType())
			trackedvars[numtrackedvars].data = var.GetPointeeData (0, dataitems);
		else
			trackedvars[numtrackedvars].data = var.GetData ();
		logprintf (LOG_NONE, "addTrackedVar: var %llx:%s tracked\n", functionaddress, varname);
		return numtrackedvars++;
	}
	logprintf (LOG_ERROR, "addTrackedVar: trackedvars size (%d) too small\n", TRACKED_VARS_MAX);
	return EOF;
}

// TODO: check why cdp & cdr changed
int
isTrackedVarChanged (addr_t functionaddress, SBValue var)
{
	if (!enable_arrays_change_detection && !global_istest)
		return 0;
	int tvi = getTrackedVar (functionaddress, var);
	if (tvi<0)
		return 0;
	int dataitems, datasize;
	getDataInfo (var, &dataitems, &datasize);
	if (datasize <= 0)
		return 0;
	dataitems = min (dataitems, ARRAY_MAX-1);
	datasize = min (datasize, ARRAY_MAX-1);
	char vbuffer[ARRAY_MAX], *pv=vbuffer;
	char tbuffer[ARRAY_MAX], *pt=tbuffer;
	vbuffer[ARRAY_MAX-1] = '\0';
	tbuffer[ARRAY_MAX-1] = '\0';
	SBType vartype = var.GetType();
	SBData data;
	if (vartype.IsPointerType())
		data = var.GetPointeeData (0, dataitems);
	else
		data = var.GetData ();
	SBError error;
	data.ReadRawData (error, 0, vbuffer, datasize);
	if (error.Fail()) {
		logprintf (LOG_WARN, "isTrackedVarChanged: ReadRawData on %s: error %s\n", var.GetName(), error.GetCString());
		return 0;
	}
	trackedvars[tvi].data.ReadRawData (error, 0, tbuffer, datasize);
	if (error.Fail()) {
		logprintf (LOG_WARN, "isTrackedVarChanged: ReadRawData on tracked %s: error %s\n", var.GetName(), error.GetCString());
		return 0;
	}
	for (int id=0; id<datasize; id++)
		if (*pv++ != *pt++) {
			trackedvars[tvi].data = data;
			++trackedvars[tvi].changes;
			logprintf (LOG_NONE, "isTrackedVarChanged: var %llx:%s changed. changes=%d\n", functionaddress, var.GetName(), trackedvars[tvi].changes);
			return trackedvars[tvi].changes;
		}
	return 0;
}


// TODO: Adjust var summary && expression not updated test sequence
// TODO: support types like struct S. seems to be a bug in lldb
bool
getPeudoArrayVariable (SBFrame frame, const char *expression, SBValue &var)
{
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
	logprintf (LOG_NONE, "getPeudoArrayVariable: expression %s -> %s\n", expression, newexpression);
	if (!var.IsValid() || var.GetError().Fail())
		return false;
	return true;
}

// from lldb-mi
SBValue
getVariable (SBFrame frame, const char *expression)
{
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
	logprintf (LOG_NONE, "getVariable: expression=%s, name=%s, value=%s, changed=%d\n",
			expression, var.GetName(), var.GetValue(), var.GetValueDidChange());
	return var;
}

// scan variables to reset their changed state
int
evaluateVarChanged (addr_t functionaddress, SBValue var, int depth)
{
	SBStream stream;											// temp
	var.GetExpressionPath(stream);								// temp
	const char *varexpressionpath = stream.GetData();			// temp
   // Force a value to update
	var.GetValue();				// get value to activate changes
	int changes = var.GetValueDidChange();
	const char *summary = var.GetSummary();			// get value to activate changes
	logprintf (LOG_NONE, "evaluateVarChanged: name=%s, varexpressionpath=%s, value=%s, summary=%s, changed=%d\n",
			var.GetName(), varexpressionpath, var.GetValue(), summary, var.GetValueDidChange());
    // And update its children
	SBType vartype = var.GetType();
	if (!vartype.IsPointerType() && !vartype.IsReferenceType() && !vartype.IsArrayType()) {
		int varnumchildren = var.GetNumChildren();
		for (int ichild = 0; ichild < varnumchildren; ++ichild) {
			SBValue child = var.GetChildAtIndex(ichild);
			if (child.IsValid() && depth>1)
				changes += evaluateVarChanged (functionaddress, child, depth-1);
		}
	}
	return changes;
}


// search changed variables
char *
formatChangedList (char *changedesc, size_t descsize, addr_t functionaddress, SBValue var, bool &separatorvisible, int depth)
{
	SBStream stream;
	var.GetExpressionPath(stream);
	const char *varexpressionpath = stream.GetData();
	if (varexpressionpath==NULL)
		return changedesc;
	logprintf (LOG_NONE, "formatChangedList: name=%s, expressionpath=%s, value=%s, summary=%s, changed=%d\n",
			var.GetName(), varexpressionpath, var.GetValue(), var.GetSummary(), var.GetValueDidChange());
	var.GetValue();					// required to get value to activate changes
	var.GetSummary();				// required to get value to activate changes
	SBType vartype = var.GetType();
	int varnumchildren = var.GetNumChildren();
	if (vartype.IsReferenceType() && varnumchildren==1) {
		// use children if reference
		var = var.GetChildAtIndex(0);
		vartype = var.GetType();
		varnumchildren = var.GetNumChildren();
	}
	int trackedvarchanges = 0;
	if (vartype.IsPointerType() || vartype.IsReferenceType() || vartype.IsArrayType())
		trackedvarchanges = isTrackedVarChanged (functionaddress, var);
	if (var.GetValueDidChange() || trackedvarchanges>0) {
		const char *separator = separatorvisible? ",":"";
		const char *varinscope = var.IsInScope()? "true": "false";
		char vardesc[NAME_MAX];
		snprintf (changedesc, descsize,
			"%s{name=\"%s\",value=\"%s\",in_scope=\"%s\",type_changed=\"false\",has_more=\"0\"}",
			separator, varexpressionpath, formatValue(vardesc,sizeof(vardesc),var,trackedvarchanges), varinscope);
		separatorvisible = true;
		return changedesc;
	}
	if (!vartype.IsPointerType() && !vartype.IsReferenceType() && !vartype.IsArrayType()) {
		for (int ichild = 0; ichild < varnumchildren; ++ichild) {
			SBValue child = var.GetChildAtIndex(ichild);
			if (!child.IsValid())
				continue;
			// Handle composite types (i.e. struct or arrays)
			int changelength = strlen(changedesc);
            if (depth>1)
            	formatChangedList (changedesc+changelength, descsize-changelength, functionaddress, child, separatorvisible, depth-1);
		}
	}
	if (strlen(changedesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatChangedList: changedesc size (%d) too small\n", descsize);
	return changedesc;
}


// format a list of variables into a GDB string
// called for arguments and locals var after a breakpoint
char *
formatVariables (char *varsdesc, size_t descsize, SBValueList varslist, addr_t functionaddress)
{
	*varsdesc = '\0';
	const char *separator="";
	for (size_t i=0; i<varslist.GetSize(); i++) {
		SBValue var = varslist.GetValueAtIndex(i);
		if (var.IsValid() && var.GetError().Success()) {
			SBType vartype = var.GetType();
			logprintf (LOG_NONE, "formatVariables: var=%s, type class=%s, basic type=%s \n",
					var.GetName(), getNameForTypeClass(vartype.GetTypeClass()), getNameForBasicType(vartype.GetBasicType()));
			const char *varvalue = var.GetValue();
			// basic type valid only when type class is Builtin
			if ((vartype.GetBasicType()!=eBasicTypeInvalid && varvalue!=NULL) || true) {
				evaluateVarChanged (functionaddress, var);
				if (vartype.IsPointerType() || vartype.IsReferenceType() || vartype.IsArrayType())
					addTrackedVar (functionaddress, var, true);		// keep var data for update changed tests
				int varslength = strlen(varsdesc);
				char vardesc[NAME_MAX*2];
				snprintf (varsdesc+varslength, descsize-varslength, "%s{name=\"%s\",value=\"%s\"}",
						separator, var.GetName(), formatValue (vardesc, sizeof(vardesc), var));
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


// format a variable description into a GDB string
char *
formatValue (char *vardesc, size_t descsize, SBValue var, int trackedvarchanges)
{
	// TODO: implement @ formats for structures
	//	value = "HFG\123klj\b"
	//	value={2,5,7,0 <repeat 5 times>, 7}
	//	value = {{a=1,b=0x33},...{a=1,b=2}}

	SBType vartype = var.GetType();
	int varnumchildren = var.GetNumChildren();
	if (vartype.IsReferenceType() && varnumchildren==1) {
		// use children if reference
		var = var.GetChildAtIndex(0);
		vartype = var.GetType();
		varnumchildren = var.GetNumChildren();
	}
	const char *varname = var.GetName();
	const char *varsummary = var.GetSummary();
	const char *varvalue = var.GetValue();
	BasicType basictype = vartype.GetBasicType();
	lldb::addr_t varaddr;
	if (vartype.IsPointerType() || vartype.IsReferenceType())
		varaddr = var.GetValueAsUnsigned();
	else
		varaddr = var.GetLoadAddress();

	logprintf (LOG_NONE, "formatValue: var=%llx, name=%s, summary=%llx:%s, value=%s, address=%llx, vartype=%s, trackedvarchanges=%d\n",
			&var, varname, varsummary, varsummary, varvalue, varaddr, getNameForBasicType(basictype), trackedvarchanges);

	*vardesc = '\0';

	if (varsummary != NULL) {		// string
		char summary[NAME_MAX];
		char *psummary = summary;
		strlcpy (summary, var.GetSummary(), sizeof(summary));
		++psummary;					// remove apostrophes
		*(psummary+strlen(psummary)-1) = '\0';
		snprintf (vardesc, descsize, "%llx \\\"%s\\\"", varaddr, psummary);
	}
	else if (varvalue != NULL) {		// number
		if (vartype.IsPointerType() || vartype.IsReferenceType() || vartype.IsArrayType()) {
			if (trackedvarchanges%2 == 0)
				snprintf (vardesc, descsize, "%s {...}", varvalue);
			else
				snprintf (vardesc, descsize, "%s {. .}", varvalue);
		}
		else		// number
			strlcpy (vardesc, varvalue, descsize);
	}
	else {
		if (trackedvarchanges%2 == 0)
			snprintf (vardesc, descsize, "%llx {...}", varaddr);
		else
			snprintf (vardesc, descsize, "%llx {. .}", varaddr);
	}
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
