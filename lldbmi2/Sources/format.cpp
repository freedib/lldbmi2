
#include "lldbmi2.h"
#include "log.h"
#include "names.h"
#include "format.h"

// Should make breakpoint pending if invalid
// 017,435 29^done,bkpt={number="5",type="breakpoint",disp="keep",enabled="y",addr="<PENDING>",pending=\
// "/Users/didier/Projets/git-lldbmi2/test_hello_c/Sources/hello.c:33",times="0",original-location="/Users/\
// didier/Projets/git-lldbmi2/test_hello_c/Sources/hello.c:33"}

// format a breakpoint description into a GDB string
char *
formatBreakpoint (char *breakpointdesc, int descsize, SBBreakpoint breakpoint, STATE *pstate)
{
	// 18^done,bkpt={number="1",type="breakpoint",disp="keep",enabled="y",addr="0x00000001000/00f58",
	//  func="main",file="../Sources/hello.c",fullname="/pro/runtime-EclipseApplication/hello/Sources/hello.c",
	//  line="17",thread-groups=["i1"],times="0",original-location="/pro/runtime-EclipseApplication/hello/Sources/hello.c:17"}
	int bpid = breakpoint.GetID();
	SBBreakpointLocation location = breakpoint.GetLocationAtIndex(0);
	SBAddress addr = location.GetAddress();
	uint32_t file_addr=addr.GetFileAddress();
	SBFunction function=addr.GetFunction();
	const char *func_name=function.GetName();
	SBLineEntry line_entry=addr.GetLineEntry();
	SBFileSpec filespec = line_entry.GetFileSpec();
	const char *filename=filespec.GetFilename();
	char filepath[PATH_MAX];
	snprintf (filepath, sizeof(filepath), "%s/%s", filespec.GetDirectory(), filename);
	uint32_t line=line_entry.GetLine();
	const char *dispose = (breakpoint.IsOneShot())? "del": "keep";
	const char *originallocation = "";
	//	originallocation,dispose = breakpoints[bpid]
	snprintf (breakpointdesc, descsize,
			"{number=\"%d\",type=\"breakpoint\",disp=\"%s\",enabled=\"y\",addr=\"0x%016x\","
			"func=\"%s\",file=\"%s\",fullname=\"%s\",line=\"%d\","
			"thread-groups=[\"%s\"],times=\"0\",original-location=\"%s\"}",
			bpid,dispose,file_addr,func_name,filename,
			filepath,line,pstate->threadgroup,originallocation);
	if (strlen(breakpointdesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatBreakpoint: breakpointdesc size (%d) too small\n", descsize);
	return breakpointdesc;
}

// get the number of frames in a thread
// adjust the number of frames if libdyld.dylib is there
int
getNumFrames (SBThread thread)
{
	int numframes=thread.GetNumFrames();
#ifndef SHOW_STARTUP
	for (int iframe=numframes-1; iframe>=0; iframe--) {
		SBFrame frame = thread.GetFrameAtIndex(iframe);
		SBModule module = frame.GetModule();
		if (module.IsValid()) {
			SBFileSpec modulefilespec = module.GetPlatformFileSpec();
			const char *modulefilename = modulefilespec.GetFilename();
			SBFunction function = frame.GetFunction();
			if (function.IsValid()) {
				const char *func_name = function.GetName();
				if (strcmp(modulefilename,"libdyld.dylib")==0 && strcmp(func_name,"start")==0)
					numframes = numframes-1;
				else
					break;
			}
			else
				break;
		}
		else
			break;
	}
#endif
	return numframes;
}

// format a frame description into a GDB string
char *
formatFrame (char *framedesc, int descsize, SBFrame frame, FrameDetails details)
{
	int frameid = frame.GetFrameID();
	SBAddress addr = frame.GetPCAddress();
	uint32_t file_addr = addr.GetFileAddress();
	SBFunction function = addr.GetFunction();
	char levelstring[NAME_MAX];
	if (details&WITH_LEVEL)
		snprintf (levelstring, sizeof(levelstring), "level=\"%d\",", frameid);
	else
		levelstring[0] = '\0';

	const char *modulefilename = "";
	SBModule module = frame.GetModule();
	if (module.IsValid()) {
		SBFileSpec modulefilespec = module.GetPlatformFileSpec();
		modulefilename = modulefilespec.GetFilename();
	}

	*framedesc = '\0';
	const char *func_name="??";
	char argsstring[LINE_MAX];
	if (function.IsValid()) {
		const char *filename, *filedir;
		int line = 0;
		func_name = function.GetName();
		SBLineEntry line_entry = addr.GetLineEntry();
		SBFileSpec filespec = line_entry.GetFileSpec();
		filename = filespec.GetFilename();
		filedir = filespec.GetDirectory();
		line = line_entry.GetLine();
		if (details&WITH_ARGS) {
			SBValueList args = frame.GetVariables(1,0,0,0);
			char argsdesc[LINE_MAX];
			formatVariables (argsdesc, sizeof(argsdesc), args);
			snprintf (argsstring, sizeof(argsstring), "%sargs=[%s]", (details==JUST_LEVEL_AND_ARGS)?"":",", argsdesc);
		}
		else
			argsstring[0] = '\0';
		if (details==JUST_LEVEL_AND_ARGS)
			snprintf (framedesc, descsize, "frame={%s%s}",
								levelstring,argsstring);
		else
			snprintf (framedesc, descsize, "frame={%saddr=\"0x%016x\",func=\"%s\"%s,file=\"%s\","
								"fullname=\"%s/%s\",line=\"%d\"}",
								levelstring,file_addr,func_name,argsstring,filename,filedir,filename,line);
	}
	else {
		if (details&WITH_ARGS)
			snprintf (argsstring, sizeof(argsstring), "%sargs=[]", (details==JUST_LEVEL_AND_ARGS)?"":",");
		else
			argsstring[0] = '\0';
		if (details==JUST_LEVEL_AND_ARGS)
			snprintf (framedesc, descsize, "frame={%s%s}",
					levelstring,argsstring);
		else {
			func_name = frame.GetFunctionName();
			snprintf (framedesc, descsize, "frame={%saddr=\"0x%016x\",func=\"%s\"%s,file=\"%s\"}",
					levelstring,file_addr,func_name, argsstring, modulefilename);
		}
	}
	if (strlen(framedesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatFrame: framedesc size (%d) too small\n", descsize);
	return framedesc;
}

// format a thread description into a GDB string
char *
formatThreadInfo (char *threaddesc, int descsize, SBProcess process, int threadindexid)
{
	*threaddesc = '\0';
	if (!process.IsValid())
		return threaddesc;
	int pid=process.GetProcessID();
	int state = process.GetState ();
	if (state == eStateStopped) {
		int tmin, tmax;
		bool useindexid;
		if (threadindexid < 0) {
			tmin = 0;
			tmax = process.GetNumThreads();
			useindexid = false;
		}
		else{
			tmin = threadindexid;
			tmax = threadindexid+1;
			useindexid = false;
		}
		const char *separator="";
		for (int ithread=tmin; ithread<tmax; ithread++) {
			SBThread thread;
			if (useindexid)
				thread = process.GetThreadByIndexID(ithread);
			else
				thread = process.GetThreadAtIndex(ithread);
			int tid=thread.GetThreadID();
			threadindexid=thread.GetIndexID();
			int frames = getNumFrames (thread);
			if (frames > 0) {
				SBFrame frame = thread.GetFrameAtIndex(0);
				if (frame.IsValid()) {
					char framedesc[LINE_MAX];
					formatFrame (framedesc, sizeof(framedesc), frame, WITH_LEVEL_AND_ARGS);
					int desclength = strlen(threaddesc);
					snprintf (threaddesc+desclength, descsize-desclength,
						"%s{id=\"%d\",target-id=\"Thread 0x%x of process %d\",%s,state=\"stopped\"}",
						separator, threadindexid, tid, pid, framedesc);
				}
			}
			separator=",";
		}
	}
	if (strlen(threaddesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatThreadInfo: threaddesc size (%d) too small\n", descsize);
	return threaddesc;
}

// TODO: references not evaluated correctly in arg list as &separatorvisible
// TODO: if char pointer, try many children (up to \0 or max 100) to check if changed, or find variable with the save address
// search changed variables
char *
formatChangedList (char *changedesc, int descsize, SBValue var, bool &separatorvisible)
{
	SBStream stream;
	var.GetExpressionPath(stream);
	const char *varname = stream.GetData();
	if (varname==NULL)
		return changedesc;
	logprintf (LOG_INFO, "formatChangedList: varname=%s, pathname=%s, value=%s, summary=%s, changed=%d\n",
			var.GetName(), varname, var.GetValue(), var.GetSummary(), var.GetValueDidChange());
	var.GetValue();					// required to get value to activate changes
	var.GetSummary();				// required to get value to activate changes
	if (var.GetValueDidChange()) {
		const char *separator = separatorvisible? ",":"";
		const char *varinscope = var.IsInScope()? "true": "false";
		char vardesc[NAME_MAX];
		snprintf (changedesc, descsize,
			"%s{name=\"%s\",value=\"%s\",in_scope=\"%s\",type_changed=\"false\",has_more=\"0\"}",
			separator, varname, formatValue(vardesc,sizeof(vardesc),var), varinscope);
		separatorvisible = true;
		return changedesc;
	}
	SBType vartype = var.GetType();
//	if (!vartype.IsPointerType() && !vartype.IsReferenceType()) {
		const int nchildren = var.GetNumChildren();
		for (int ichildren = 0; ichildren < nchildren; ++ichildren) {
			SBValue member = var.GetChildAtIndex(ichildren);
			if (!member.IsValid())
				continue;
				// Handle composite types (i.e. struct or arrays)
			int changelength = strlen(changedesc);
			formatChangedList (changedesc+changelength, descsize-changelength, member, separatorvisible);
		}
//	}
	if (strlen(changedesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatChangedList: changedesc size (%d) too small\n", descsize);
	return changedesc;
}


int
countChangedList (SBValue var)
{
    // Force a value to update
	var.GetValue();				// get value to activate changes
	var.GetSummary();			// get value to activate changes
	int changes = var.GetValueDidChange();
	logprintf (LOG_NONE, "resetChangedList: name=%s, value=%s, changed=%d\n",
			var.GetName(), var.GetValue(), var.GetValueDidChange());
    // And update its children
	SBType vartype = var.GetType();
//	if (!vartype.IsPointerType() && !vartype.IsReferenceType()) {
		const int nchildren = var.GetNumChildren();
		for (int ichildren = 0; ichildren < nchildren; ++ichildren) {
			SBValue member = var.GetChildAtIndex(ichildren);
            if (member.IsValid())
            	changes += countChangedList (member);
        }
//	}
	return changes;
}

#define min(a,b) ((a) < (b) ? (a) : (b))

// TODO: Adjust var summary && expression not updated test sequence
// TODO: support types like struct S
bool
getPeudoArrayVariable (SBFrame frame, const char *expression, SBValue &var)
{
// just support *((var)+0)@100 form
// char c[101];
// -var-create * *((c)+0)@100
// -var-create * *((c)+100)@1
	char *pe, e[NAME_MAX], *varname, *varoffset, *varlength, varpointername[NAME_MAX], vartype[NAME_MAX];
	strlcpy (e, expression, sizeof(e));
	pe = e;
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
	snprintf (varpointername, sizeof(varpointername), "&%s", varname);	// get var type
	SBValue basevar = getVariable (frame, varpointername);				// vaname ¬ &var
	char newvartype[NAME_MAX];
	strlcpy (newvartype, basevar.GetTypeName(), sizeof(vartype));		// typename ~ char (*)[101]
	if ((pe = strchr (newvartype,'[')) == NULL)
		return false;
	*pe = '\0';
	char newexpression[NAME_MAX];				// create expression
	snprintf (newexpression, sizeof(newexpression), "(%s[%s])&%s[%s]",	// (char(*)[100])&c[0]
			newvartype, varlength, varname, varoffset);
	var = getVariable (frame, newexpression);
 	logprintf (LOG_INFO, "getPeudoArrayVariable: expression %s -> %s\n", expression, newexpression);
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
	countChangedList (var);		// to update ValueDidChange
 	logprintf (LOG_INFO, "getVariable: fullname=%s, name=%s, value=%s, changed=%d\n",
			expression, var.GetName(), var.GetValue(), var.GetValueDidChange());
//	var.SetPreferDynamicValue(eDynamicDontRunTarget);
	return var;
}


// format a list of variables into a GDB string
char *
formatVariables (char *varsdesc, int descsize, SBValueList varslist)
{
	*varsdesc = '\0';
	const char *separator="";
	for (int i=0; i<varslist.GetSize(); i++) {
		SBValue var = varslist.GetValueAtIndex(i);
		if (var.IsValid() && var.GetError().Success()) {
			const char *ss = var.GetSummary();
			logprintf (LOG_INFO, "formatVariables: summary=%s\n", ss);
			BasicType basictype = var.GetType().GetBasicType();
			const char *varvalue = var.GetValue();
		//	resetChangedList (var);
			if ((basictype!=eBasicTypeInvalid && varvalue!=NULL) || true) {
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
formatValue (char *vardesc, int descsize, SBValue var)
{
	const char *varname = var.GetName();
	const char *varsummary = var.GetSummary();
	const char *varvalue = var.GetValue();
	BasicType basictype = var.GetType().GetBasicType();
	logprintf (LOG_NONE, "formatValue: varname=%s, varsummary=%s, varvalue=%s, vartype=%s\n",
			varname, varsummary, varvalue, getNameForBasicType(basictype));

	*vardesc = '\0';
	if (varsummary != NULL) {		// string
		char summary[NAME_MAX];
		char *psummary = summary;
		strlcpy (summary, var.GetSummary(), sizeof(summary));
		++psummary;
		*(psummary+strlen(psummary)-1) = '\0';
		snprintf (vardesc, descsize, "%s \\\"%s\\\"", var.GetValue(), psummary);
	}
	else if (varvalue != NULL)		// number
		strlcpy (vardesc, varvalue, descsize);
	else
		snprintf (vardesc, descsize, "{...}");
	if (strlen(vardesc) >= descsize-1)
		logprintf (LOG_ERROR, "formatValue: vardesc size (%d) too small\n", descsize);
	return vardesc;
}

