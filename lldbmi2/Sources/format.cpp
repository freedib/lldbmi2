
#include "lldbmi2.h"
#include "log.h"
#include "names.h"
#include "format.h"


// TODO: make pending si bp invalid
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
		logprintf (LOG_ERROR, "breakpointdesc size (%d) too small\n", descsize);
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
		logprintf (LOG_ERROR, "framedesc size (%d) too small\n", descsize);
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
		logprintf (LOG_ERROR, "threaddesc size (%d) too small\n", descsize);
	return threaddesc;
}


// search changed vars
char *
formatChangedList (char *changedesc, int descsize, char *fullname, int namesize, SBValue var, bool &separatorvisible)
{
	const char *varname = var.GetName();
	if (*fullname!='\0' && *varname!='[')
		strlcat (fullname, ".", namesize);
	strlcat (fullname, varname, namesize);
	namesize -= sizeof(varname);
	logprintf (LOG_INFO, "formatChangedList: varname=%s, fullname=%s, changed=%d\n",
			var.GetName(), fullname, var.GetValueDidChange());
	if (var.GetValueDidChange()) {
		const char *separator = separatorvisible? ",":"";
		const char *varinscope = var.IsInScope()? "true": "false";
		char vardesc[NAME_MAX];
		snprintf (changedesc, descsize,
			"%s{name=\"%s\",value=\"%s\",in_scope=\"%s\",type_changed=\"false\",has_more=\"0\"}",
			separator, fullname, formatValue(vardesc,sizeof(vardesc),var), varinscope);
		separatorvisible = true;
		return changedesc;
	}
	SBType vartype = var.GetType();
	if (!vartype.IsPointerType() && !vartype.IsReferenceType()) {
		const int nchildren = var.GetNumChildren();
		for (int ichildren = 0; ichildren < nchildren; ++ichildren) {
			SBValue member = var.GetChildAtIndex(ichildren);
			if (!member.IsValid())
				continue;
				// Handle composite types (i.e. struct or arrays)
			int changelength = strlen(changedesc);
			int namelength = strlen(fullname);
			formatChangedList (changedesc+changelength, descsize-changelength,
					fullname, namesize, member, separatorvisible);
			fullname[namelength] = '\0';	// reset the name to the parent
		}
	}
	if (strlen(changedesc) >= descsize-1)
		logprintf (LOG_ERROR, "varsdesc size (%d) too small\n", descsize);
	return changedesc;
}

void
resetChangedList (SBValue var)
{
    // Force a value to update
    var.GetValueDidChange();
    // And update its children
	SBType vartype = var.GetType();
	if (!vartype.IsPointerType() && !vartype.IsReferenceType()) {
		const int nchildren = var.GetNumChildren();
		for (int ichildren = 0; ichildren < nchildren; ++ichildren) {
			SBValue member = var.GetChildAtIndex(ichildren);
            if (member.IsValid())
            	resetChangedList (member);
        }
    }
}

SBValue
createVariable (SBFrame frame, const char *expression)
{
	SBValue var;
    if (expression[0] == '$')
    	var = frame.FindRegister(expression);
    else {
        const bool bArgs = true;
        const bool bLocals = true;
        const bool bStatics = true;
        const bool bInScopeOnly = false;
        const SBValueList varslist = frame.GetVariables (bArgs, bLocals, bStatics, bInScopeOnly);
        var = varslist.GetFirstValueByName (expression);
    }

    if (!var.IsValid()) {
    	var = frame.EvaluateExpression (expression);
    }
    if (var.IsValid() && var.GetError().Success())
    	resetChangedList (var);
    return var;
}



// find a variable or expression
//   look first in variable to avoid create a new expression
SBValue
getVariable (SBFrame frame, const char *expression)
{
	SBValue var;
/*
	if (true) {
		var = frame.GetValueForVariablePath(expression);
		if (var.IsValid() && !var.GetError().Fail())
			logprintf (LOG_INFO, "GetValueForVariablePath(%s): varname=%s, varsummary=%s, varvalue=%s, vartype=%s\n",
					expression, var.GetName(), var.GetSummary(), var.GetValue(), getNameForBasicType(var.GetType().GetBasicType()));
		else
			logprintf (LOG_INFO, "GetValueForVariablePath(%s): invalid\n",	expression);
		var = frame.FindVariable(expression);
		if (var.IsValid() && !var.GetError().Fail())
			logprintf (LOG_INFO, "FindVariable(%s): varname=%s, varsummary=%s, varvalue=%s, vartype=%s\n",
					expression, var.GetName(), var.GetSummary(), var.GetValue(), getNameForBasicType(var.GetType().GetBasicType()));
		else
			logprintf (LOG_INFO, "FindVariable(%s): invalid\n",	expression);
		var = frame.EvaluateExpression(expression);
		if (var.IsValid() && !var.GetError().Fail())
			logprintf (LOG_INFO, "EvaluateExpression(%s): varname=%s, varsummary=%s, varvalue=%s, vartype=%s\n",
					expression, var.GetName(), var.GetSummary(), var.GetValue(), getNameForBasicType(var.GetType().GetBasicType()));
		else
			logprintf (LOG_INFO, "EvaluateExpression(%s): invalid\n",	expression);
	}
*/
	var = frame.GetValueForVariablePath(expression);
	if (!var.IsValid() || var.GetError().Fail())
		var = frame.EvaluateExpression(expression);
	if (var.IsValid() && !var.GetError().Fail())
		logprintf (LOG_INFO, "getVariable: success expr=%s, name-%s, value=%s\n",
				expression, var.GetName(), var.GetValue());
	else
		logprintf (LOG_INFO, "getVariable: error expr=%s name-%s, value=%s\n",
				expression, var.GetName(), var.GetValue());
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
		if (var.IsValid() && !var.GetError().Fail()) {
			BasicType basictype = var.GetType().GetBasicType();
			const char *varvalue = var.GetValue();
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
		logprintf (LOG_ERROR, "varsdesc size (%d) too small\n", descsize);
	return varsdesc;
}

// format a variable description into a GDB string
char *
formatValue (char *vardesc, int descsize, SBValue var)
{
//	const char *varname = var.GetName();
	const char *varsummary = var.GetSummary();
	const char *varvalue = var.GetValue();
//	BasicType basictype = var.GetType().GetBasicType();

//	logprintf (LOG_INFO, "formatValue: varname=%s, varsummary=%s, varvalue=%s, vartype=%s\n",
//			varname, varsummary, varvalue, getNameForBasicType(basictype));

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
		logprintf (LOG_ERROR, "vardesc size (%d) too small\n", descsize);
	return vardesc;
}

