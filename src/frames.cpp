
#include "lldbmi2.h"
#include "log.h"
#include "frames.h"
#include "variables.h"
#include "names.h"


// get the number of frames in a thread
// adjust the number of frames if libdyld.dylib is there
int
getNumFrames (SBThread thread)
{
	logprintf (LOG_TRACE, "getNumFrames(0x%x)\n", &thread);
	int numframes=thread.GetNumFrames();
	logprintf (LOG_DEBUG, "getNumFrames(0x%x) = %d\n", &thread, numframes);
	return numframes;
}


// Should make breakpoint pending if invalid
// 017,435 29^done,bkpt={number="5",type="breakpoint",disp="keep",enabled="y",addr="<PENDING>",pending=
//     "/project_path/test_hello_c/Sources/tests.cpp:33",times="0",original-location=
//     "/project_path/test_hello_c/Sources/tests.cpp:33"}

// format a breakpoint description into a GDB string
char *
formatBreakpoint (SBBreakpoint breakpoint, STATE *pstate)
{
	static StringB breakpointdescB(LINE_MAX);
	breakpointdescB.clear();
	return formatBreakpoint (breakpointdescB, breakpoint, pstate);
}

char *
formatBreakpoint (StringB &breakpointdescB, SBBreakpoint breakpoint, STATE *pstate)
{
	logprintf (LOG_TRACE, "formatBreakpoint (0x%x, 0x%x, 0x%x)\n", &breakpointdescB, &breakpoint, pstate);
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
	breakpointdescB.catsprintf (
			"{number=\"%d\",type=\"breakpoint\",disp=\"%s\",enabled=\"y\",addr=\"0x%016x\","
			"func=\"%s\",file=\"%s\",fullname=\"%s\",line=\"%d\","
			"thread-groups=[\"%s\"],times=\"0\",original-location=\"%s\"}",
			bpid,dispose,file_addr,func_name,filename,
			filepath,line,pstate->threadgroup,originallocation);
	return breakpointdescB.c_str();
}


// format a frame description into a GDB string
// format a frame description into a GDB string
char *
formatFrame (SBFrame frame, FrameDetails framedetails)
{
	static StringB framedescB(LINE_MAX);
	framedescB.clear();
	return formatFrame (framedescB, frame, framedetails);
}

char *
formatFrame (StringB &framedescB, SBFrame frame, FrameDetails framedetails)
{
	logprintf (LOG_TRACE, "formatFrame (0x%x, 0x%x, 0x%x)\n", &framedescB, &frame, framedetails);
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

	const char *func_name="??";
	static StringB argsstringB(LINE_MAX);
	argsstringB.clear();
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
			static StringB argsdescB(LINE_MAX);
			argsdescB.clear();
			SBFunction function = frame.GetFunction();
			formatVariables (argsdescB, args);
			argsstringB.catsprintf ("%sargs=[%s]", (framedetails==JUST_LEVEL_AND_ARGS)?"":",", argsdescB.c_str());
		}
		if (framedetails==JUST_LEVEL_AND_ARGS)
			framedescB.catsprintf ("frame={%s%s}", levelstring, argsstringB.c_str());
		else
			framedescB.catsprintf ("frame={%saddr=\"0x%016x\",func=\"%s\"%s,file=\"%s\","
								"fullname=\"%s/%s\",line=\"%d\"}",
								levelstring,file_addr,func_name,argsstringB.c_str(),filename,filedir,filename,line);
	}
	else {
		if (framedetails&WITH_ARGS)
			argsstringB.catsprintf ("%sargs=[]", (framedetails==JUST_LEVEL_AND_ARGS)?"":",");
		if (framedetails==JUST_LEVEL_AND_ARGS)
			framedescB.catsprintf ("frame={%s%s}", levelstring, argsstringB.c_str());
		else {
			func_name = frame.GetFunctionName();
			framedescB.catsprintf ("frame={%saddr=\"0x%016x\",func=\"%s\"%s,file=\"%s\"}",
					levelstring, file_addr, func_name, argsstringB.c_str(), modulefilename);
		}
	}
	return framedescB.c_str();
}


// format a thread description into a GDB string
char *
formatThreadInfo (SBProcess process, int threadindexid)
{
	static StringB threaddescB(LINE_MAX);
	threaddescB.clear();
	return formatThreadInfo (threaddescB, process, threadindexid);
}

char *
formatThreadInfo (StringB &threaddescB, SBProcess process, int threadindexid)
{
	logprintf (LOG_TRACE, "formatThreadInfo (0x%x, 0x%x, %d)\n", &threaddescB, &process, threadindexid);
	threaddescB.clear();
	if (!process.IsValid())
		return threaddescB.c_str();
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
					char * framedescstr = formatFrame (frame, WITH_LEVEL_AND_ARGS);
					threaddescB.catsprintf (
						"%s{id=\"%d\",target-id=\"Thread 0x%x of process %d\",%s,state=\"stopped\"}",
						separator, threadindexid, tid, pid, framedescstr);
				}
			}
			separator=",";
		}
	}
	return threaddescB.c_str();
}
