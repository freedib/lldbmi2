
#include "lldbmi2.h"
#include "format.h"
#include "log.h"


// format a breakpoint description into a GDB string
const char *
formatbreakpoint (char *breakpointdesc, int descsize, SBBreakpoint breakpoint, STATE *pstate)
{
	// 18^done,bkpt={number="1",type="breakpoint",disp="keep",enabled="y",addr="0x00000001000/00f58",
	//  func="main",file="../src/hello.c",fullname="/pro/runtime-EclipseApplication/hello/src/hello.c",
	//  line="17",thread-groups=["i1"],times="0",original-location="/pro/runtime-EclipseApplication/hello/src/hello.c:17"}
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
	return breakpointdesc;
}

// get the number of frames in a thread
// adjust the number of frames if libdyld.dylib is there
int getNumFrames (SBThread thread)
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
const char *
formatframe (char *framedesc, int descsize, SBFrame frame, bool withlevel)
{
	int frameid = frame.GetFrameID();
	SBAddress addr = frame.GetPCAddress();
	uint32_t file_addr = addr.GetFileAddress();
	SBFunction function = addr.GetFunction();
	char levelstring[NAME_MAX];
	if (withlevel)
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
	const char *func_name="???";
	if (function.IsValid()) {
		const char *filename, *filedir;
		int line = 0;
		func_name = function.GetName();
		SBLineEntry line_entry = addr.GetLineEntry();
		SBFileSpec filespec = line_entry.GetFileSpec();
		filename = filespec.GetFilename();
		filedir = filespec.GetDirectory();
		line = line_entry.GetLine();
		SBValueList args = frame.GetVariables(1,0,0,0);
		char argsdesc[LINE_MAX];
		formatvariables (argsdesc, sizeof(argsdesc), args);
		snprintf (framedesc, descsize, "frame={%saddr=\"0x%016x\",func=\"%s\",args=[%s],file=\"%s\","
							"fullname=\"%s/%s\",line=\"%d\"}",
							levelstring,file_addr,func_name,argsdesc,filename,filedir,filename,line);
	}
	else {
		bool show_system_dylib_names = true;		// standard GDB = false
		func_name = frame.GetFunctionName();
		if (function.IsValid() && show_system_dylib_names)
			snprintf (framedesc, descsize, "frame={%saddr=\"0x%016x\",func=\"%s\",args=[],file=\"%s\"}",
					levelstring,file_addr,func_name, modulefilename);
		else
			snprintf (framedesc, descsize, "frame={%saddr=\"0x%016x\",func=\"%s\"}",
					levelstring,file_addr,func_name);
	}
	return framedesc;
}

/// find a variable or expression
///   look first in variable to avoid create a new expression
SBValue
getVariable (SBFrame frame, const char *expression)
{
	SBValue var;
	int tries=1;
	var = frame.GetValueForVariablePath(expression);
	if (!var.IsValid())	{
		++tries;
		var = frame.FindVariable(expression);
		if (!var.IsValid()) {
			++tries;
			var = frame.EvaluateExpression(expression);
		}
	}
	if (var.IsValid())
		logprintf (LOG_INFO, "getVariable: tries=%d, name-%s, value=%s\n", tries,var.GetName(), var.GetValue());
	return var;
}

// format a list of variables into a GDB string
const char *
formatvariables (char *varsdesc, int descsize, SBValueList varslist)
{
	char vardesc[NAME_MAX];
	*varsdesc = '\0';
	const char *separator="";
	for (int i=0; i<varslist.GetSize(); i++) {
		SBValue var = varslist.GetValueAtIndex(i);
		if (var.IsValid()) {
			int varslength = strlen(varsdesc);
			snprintf (varsdesc+varslength, descsize-varslength, "%s{name=\"%s\",value=\"%s\"}",
					separator, var.GetName(), formatvalue (vardesc, sizeof(vardesc), var));
			separator=",";
		}
	}
	return varsdesc;
}

// format a variable description into a GDB string
const char *
formatvalue (char *vardesc, int descsize, SBValue var)
{
	const char *varsummary = var.GetSummary();
	const char *varvalue = var.GetValue();
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
		snprintf (vardesc, descsize, "%s <%s>", var.GetValue(), var.GetName());
	return vardesc;
}

// format a thread description into a GDB string
const char *
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
					formatframe (framedesc, sizeof(framedesc), frame, true);
					int desclength = strlen(threaddesc);
					snprintf (threaddesc+desclength, descsize-desclength,
						"%s{id=\"%d\",target-id=\"Thread 0x%x of process %d\",%s,state=\"stopped\"}",
						separator, threadindexid, tid, pid, framedesc);
				}
			}
			separator=",";
		}
	}
	return threaddesc;
}
