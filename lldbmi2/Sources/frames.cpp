
#include "lldbmi2.h"
#include "log.h"
#include "frames.h"
#include "variables.h"
#include "names.h"


extern LIMITS limits;


// Should make breakpoint pending if invalid
// 017,435 29^done,bkpt={number="5",type="breakpoint",disp="keep",enabled="y",addr="<PENDING>",pending=\
// "/project_path/test_hello_c/Sources/tests.cpp:33",times="0",original-location=\
// "/project_path/test_hello_c/Sources/tests.cpp:33"}

// format a breakpoint description into a GDB string
char *
formatBreakpoint (char *breakpointdesc, size_t descsize, SBBreakpoint breakpoint, STATE *pstate)
{
	logprintf (LOG_TRACE, "formatBreakpoint (..., %d, 0x%x, 0x%x)\n", descsize, &breakpoint, pstate);
	// 18^done,bkpt={number="1",type="breakpoint",disp="keep",enabled="y",addr="0x00000001000/00f58",
	//  func="main",file="../Sources/tests.cpp",fullname="/pro/runtime-EclipseApplication/tests/Sources/tests.cpp",
	//  line="17",thread-groups=["i1"],times="0",original-location="/pro/runtime-EclipseApplication/tests/Sources/tests.cpp:17"}
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
	logprintf (LOG_TRACE, "getNumFrames (0x%x)\n", &thread);
	int numframes=thread.GetNumFrames();
#ifndef SHOW_STARTUP
	for (int iframe=min(numframes-1,limits.frames_max); iframe>=0; iframe--) {
		SBFrame frame = thread.GetFrameAtIndex(iframe);
		if (!frame.IsValid())
			continue;
		SBModule module = frame.GetModule();
		if (!module.IsValid())
			break;
		SBFileSpec modulefilespec = module.GetPlatformFileSpec();
		const char *modulefilename = modulefilespec.GetFilename();
		SBFunction function = frame.GetFunction();
		if (!function.IsValid())
			break;
		const char *func_name = function.GetName();
		if (strcmp(modulefilename,"libdyld.dylib")!=0 || strcmp(func_name,"start")!=0)
			break;
		numframes = numframes-1;
	}
#endif
	return numframes;
}

// format a frame description into a GDB string
char *
formatFrame (char *framedesc, size_t descsize, SBFrame frame, FrameDetails framedetails)
{
	logprintf (LOG_TRACE, "formatFrame (..., %d, 0x%x, 0x%x)\n", descsize, &frame, framedetails);
	int frameid = frame.GetFrameID();
	SBAddress addr = frame.GetPCAddress();
	uint32_t file_addr = addr.GetFileAddress();
	SBFunction function = addr.GetFunction();
	char levelstring[NAME_MAX];
	if (framedetails&WITH_LEVEL)
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
	char argsstring[BIG_LINE_MAX];
	if (function.IsValid()) {
		const char *filename, *filedir;
		int line = 0;
		func_name = function.GetName();
		SBLineEntry line_entry = addr.GetLineEntry();
		SBFileSpec filespec = line_entry.GetFileSpec();
		filename = filespec.GetFilename();
		filedir = filespec.GetDirectory();
		line = line_entry.GetLine();
		if (framedetails&WITH_ARGS) {
			SBValueList args = frame.GetVariables(1,0,0,0);
			char argsdesc[BIG_LINE_MAX];
			SBFunction function = frame.GetFunction();
			formatVariables (argsdesc, sizeof(argsdesc), args);
			snprintf (argsstring, sizeof(argsstring), "%sargs=[%s]", (framedetails==JUST_LEVEL_AND_ARGS)?"":",", argsdesc);
		}
		else
			argsstring[0] = '\0';
		if (framedetails==JUST_LEVEL_AND_ARGS)
			snprintf (framedesc, descsize, "frame={%s%s}",
								levelstring,argsstring);
		else
			snprintf (framedesc, descsize, "frame={%saddr=\"0x%016x\",func=\"%s\"%s,file=\"%s\","
								"fullname=\"%s/%s\",line=\"%d\"}",
								levelstring,file_addr,func_name,argsstring,filename,filedir,filename,line);
	}
	else {
		if (framedetails&WITH_ARGS)
			snprintf (argsstring, sizeof(argsstring), "%sargs=[]", (framedetails==JUST_LEVEL_AND_ARGS)?"":",");
		else
			argsstring[0] = '\0';
		if (framedetails==JUST_LEVEL_AND_ARGS)
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
formatThreadInfo (char *threaddesc, size_t descsize, SBProcess process, int threadindexid)
{
	logprintf (LOG_TRACE, "formatThreadInfo (..., %d, 0x%x, %d)\n", descsize, &process, threadindexid);
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
			if (!thread.IsValid())
				continue;
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

