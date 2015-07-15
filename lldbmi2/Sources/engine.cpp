
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "lldbmi2.h"
#include "events.h"
#include "format.h"
#include "log.h"
#include "names.h"
#include "engine.h"


void initializeSB (STATE *pstate)
{
	SBDebugger::Initialize();
	pstate->debugger = SBDebugger::Create();
	pstate->debugger.SetAsync (true);
	pstate->listener = pstate->debugger.GetListener();
}

void terminateSB ()
{
	waitProcessListener ();
	SBDebugger::Terminate();
}


// command interpreter
//   decode the line in input
//   execute the command
//   respond on stdout
int
fromCDT (STATE *pstate, const char *line, int linesize)			// from cdt
{
	char *endofline;
	char cdtline[LINE_MAX];			// must be the same size as state.cdtbuffer
	int dataflag;
	char programpath[LINE_MAX];
	int nextarg;
	CDT_COMMAND cc;

	static SBTarget target;
	static SBLaunchInfo launchInfo(NULL);
	static SBProcess process;

	dataflag = MORE_DATA;
	logdata (LOG_CDT_IN|LOG_RAW, line, strlen(line));
	strlcat (pstate->cdtbuffer, line, sizeof(pstate->cdtbuffer));
	if (pstate->lockcdt)
		return WAIT_DATA;
	if ((endofline=strstr(pstate->cdtbuffer, "\n")) != NULL) {
		// multiple command in cdtbuffer. take the first one and shify the buffer
		strncpy (cdtline, pstate->cdtbuffer, endofline+1-pstate->cdtbuffer);
		cdtline[endofline+1-pstate->cdtbuffer] = '\0';
		memmove (pstate->cdtbuffer, endofline+1, strlen(endofline));	// shift buffered data
		if (pstate->cdtbuffer[0]=='\0')
			dataflag = WAIT_DATA;
		// remove trailing \r and \n
		endofline=strstr(cdtline, "\n");
		while (endofline>cdtline && (*(endofline-1)=='\n' || *(endofline-1)=='\r'))
			--endofline;
		*endofline = '\0';
		logdata (LOG_CDT_IN, cdtline, strlen(cdtline));
	}
	else
		return WAIT_DATA;

	nextarg = evalCDTLine (pstate, cdtline, &cc);

	if (nextarg==0) {
	}
	// MISCELLANOUS COMMANDS
	else if (strcmp (cc.argv[0],"-gdb-exit")==0) {
		terminateProcess (pstate, AND_EXIT);
	}
	else if (strcmp(cc.argv[0],"-gdb-version")==0) {
		cdtprintf ("~\"%s\"\n", pstate->gdbPrompt);
		cdtprintf ("~\"%s\"\n", pstate->lldbmi2Prompt);
		cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-list-features")==0) {
		cdtprintf ("%d^done,%s\n(gdb)\n", cc.sequence,
			"features=[\"frozen-varobjs\",\"pending-breakpoints\",\"thread-info\",\"data-read-memory-bytes\",\"breakpoint-notifications\",\"ada-task-info\",\"python\"]");
	}
	else if (strcmp(cc.argv[0],"-environment-cd")==0) {
		// environment-cd /Users/didier/Projets/LLDB/hello
		launchInfo.SetWorkingDirectory (cc.argv[nextarg]);
	//	logprintf (LOG_THREAD, "cwd=%s pwd=%s\n", cc.argv[nextarg], launchInfo.GetWorkingDirectory());
		cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-gdb-set")==0) {
		// gdb-set args ...
		// gdb-set breakpoint pending on
		// gdb-set detach-on-fork on
		// gdb-set python print-stack none
		// gdb-set print object on
		// gdb-set print sevenbit-strings on
		// gdb-set host-charset UTF-8
		// gdb-set target-charset US-ASCII
		// gdb-set target-wide-charset UTF-32
		// gdb-set target-async off
		// gdb-set auto-solib-add on
		// gdb-set language c
		if (strcmp(cc.argv[nextarg],"args") == 0) {
			launchInfo.SetArguments (&cc.argv[++nextarg], false);
		}
		else if (strcmp(cc.argv[nextarg],"env") == 0) {
			// eclipse put a space around the equal in VAR = value. we have to combine all 3 part to form env entry
			static char enventry[LINE_MAX];
			const char *env[] = {enventry, NULL};
			const char **envp = env;
			++nextarg;
			if (cc.argc+1-nextarg > 0) {
				if (cc.argc+1-nextarg == 1)
					snprintf (enventry, sizeof(enventry), "%s", cc.argv[nextarg]);
				else if (cc.argc+1-nextarg == 2)
					snprintf (enventry, sizeof(enventry), "%s%s", cc.argv[nextarg], cc.argv[nextarg+1]);
				else
					snprintf (enventry, sizeof(enventry), "%s%s%s", cc.argv[nextarg], cc.argv[nextarg+1], cc.argv[nextarg+2]);
				launchInfo.SetEnvironmentEntries (envp, false);
			}
		}
		cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-gdb-show")==0) {
		// 21-gdb-show --thread-group i1 language
		SBThread thread = process.GetSelectedThread();
		if (thread.IsValid()) {
			SBFrame frame = thread.GetSelectedFrame();
			SBCompileUnit compileunit = frame.GetCompileUnit();
			LanguageType languagetype = compileunit.GetLanguage();
			const char *languagename = getNameForLanguageType(languagetype);
			cdtprintf ("%d^done,value=\"%s\"\n(gdb)\n", cc.sequence, languagename);
		}
		else
			cdtprintf ("%d^done,value=\"auto\"\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-enable-pretty-printing")==0) {
		cdtprintf ("%d^done\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"source")==0) {
		// source .gdbinit
		cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-inferior-tty-set")==0) {
		// inferior-tty-set --thread-group i1 /dev/ttyp0
		strlcpy (pstate->cdtptyname, cc.argv[nextarg], sizeof(pstate->cdtptyname));
		if (strcmp(pstate->cdtptyname,"%1") == 0)
			pstate->ptyfd = EOF;
		else
			pstate->ptyfd = open (pstate->cdtptyname, O_RDWR);
		logprintf (LOG_NONE, "pty = %d\n", pstate->ptyfd);
		cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
	}
	// TARGET AND RUN COMMANDS
	else if (strcmp(cc.argv[0],"-file-exec-and-symbols")==0) {
		// file-exec-and-symbols --thread-group i1 /Users/didier/Projets/LLDB/hello/Debug/hello
		strlcpy (programpath, cc.argv[nextarg], sizeof(programpath));
		target = pstate->debugger.CreateTargetWithFileAndArch (programpath, "x86_64");
		if (!target.IsValid())
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
		else
			cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-target-attach")==0) {
		// target-attach --thread-group i1 40088
		// =thread-group-started,id="i1",pid="40123"
		// =thread-created,id="1",group-id="i1"
		int pid=0;
		SBError error;
		char processname[PATH_MAX];
		processname[0] = '\0';
		if (cc.argv[nextarg] != NULL) {
			if (isdigit(*cc.argv[nextarg]))
				sscanf (cc.argv[nextarg], "%d", &pid);
			else
				strlcpy (processname, cc.argv[nextarg], PATH_MAX);
		}
		target = pstate->debugger.CreateTarget (NULL);
	//	pstate->debugger.SetAsync (false);
		if (pid > 0)
			process = target.AttachToProcessWithID (pstate->listener, pid, error);
		else if (processname[0]!='\0')
			process = target.AttachToProcessWithName (pstate->listener, processname, false, error);
	//	pstate->debugger.SetAsync (true);
		if (!process.IsValid()) {
			cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "Can not start process.");
			logprintf (LOG_INFO, "process_error=%s\n", error.GetCString());
		}
		else {
			pstate->isrunning = true;
			pstate->process = process;
			startProcessListener (pstate);
			setSignals (pstate);
			SBThread thread = process.GetSelectedThread();
			cdtprintf ("=thread-group-started,id=\"%s\",pid=\"%lld\"\n", pstate->threadgroup, process.GetProcessID());
			checkThreadsLife (pstate, process);
			cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
		}
	}
	else if (strcmp(cc.argv[0],"-target-detach")==0) {
		// target-detach --thread-group i1
		if (process.IsValid()) {
			terminateProcess (pstate, PRINT_THREAD|PRINT_GROUP|AND_EXIT);
			cdtprintf (	"%d^done\n(gdb)\n",	cc.sequence);
		}
		else
			cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "The program is not being run.");

	}
	else if (strcmp(cc.argv[0],"-exec-run")==0) {
		// exec-run --thread-group i1
		logprintf (LOG_NONE, "launchInfo: args=%d env=%d, pwd=%s\n", launchInfo.GetNumArguments(), launchInfo.GetNumEnvironmentEntries(), launchInfo.GetWorkingDirectory());
		SBError error;
		process = target.Launch (launchInfo, error);
		if (!process.IsValid()) {
			cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "Can not start process.");
			logprintf (LOG_INFO, "process_error=%s\n", error.GetCString());
		}
		else {
			pstate->isrunning = true;
			pstate->process = process;
			startProcessListener(pstate);
			setSignals (pstate);
			SBThread thread = process.GetSelectedThread();
			cdtprintf ("=thread-group-started,id=\"%s\",pid=\"%lld\"\n", pstate->threadgroup, process.GetProcessID());
			checkThreadsLife (pstate, process);
			cdtprintf ("%d^running\n", cc.sequence);
			cdtprintf ("*running,thread-id=\"all\"\n(gdb)\n");
		}
	}
	else if (strcmp(cc.argv[0],"-exec-continue")==0) {
		// 37-exec-continue --thread 1
		// 37^running
		// *running,thread-id="1"
		// Ignore a --thread argument. restart all threads
		if (process.IsValid()) {
			int state = process.GetState ();
			if (state == eStateStopped) {
				SBThread thread = process.GetSelectedThread();
				cdtprintf ("%d^running\n", cc.sequence);
				cdtprintf ("*running,thread-id=\"%d\"\n(gdb)\n", thread.GetIndexID());
				process.Continue();
				pstate->isrunning = true;
			}
		}
		else
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-exec-step")==0 || strcmp(cc.argv[0],"-exec-next")==0) {
		// 37-exec-next --thread 1 1
		// 37-exec-step --thread 1 1
		// 37^running
		// *running,thread-id="1"
		int times=1;		// not used for now
		if (cc.argv[nextarg] != NULL)
			if (isdigit(*cc.argv[nextarg]))
				sscanf (cc.argv[nextarg], "%d", &times);
		if (process.IsValid()) {
			int state = process.GetState ();
			if (state == eStateStopped) {
				SBThread thread = process.GetSelectedThread();
				if (thread.IsValid()) {
					cdtprintf ("%d^running\n", cc.sequence);
					cdtprintf ("*running,thread-id=\"%d\"\n(gdb)\n", thread.GetIndexID());
					if (strcmp(cc.argv[0],"-exec-step")==0)
						thread.StepInto();
					else
						thread.StepOver();
				}
				else
					cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
			}
		}
		else
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-exec-finish")==0) {
		// 37-exec-finish --thread 1 --frame 0
		// 37^running
		// *running,thread-id="all"
		if (process.IsValid()) {
			int state = process.GetState ();
			if (state == eStateStopped) {
				SBThread thread = process.GetSelectedThread();
				if (thread.IsValid()) {
					cdtprintf ("%d^running\n", cc.sequence);
					cdtprintf ("*running,thread-id=\"all\"\n(gdb)\n");
					thread.StepOut();
				}
				else
					cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
			}
		}
		else
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-interpreter-exec")==0) {
		//18-interpreter-exec --thread-group i1 console "show endian"
		//   ~"The target endianness is set automatically (currently little endian)\n"
		//18-interpreter-exec --thread-group i1 console "p/x (char)-1"
		//   ~"$1 = 0xff\n"
		//30-interpreter-exec --thread-group i1 console kill
		//   =thread-exited,id="1",group-id="i1"
		//   =thread-group-exited,id="i1"
		//   30^done
		//   (gdb)
		if (strcmp(cc.argv[nextarg],"console") == 0) {
			if (++nextarg>=cc.argc)
				cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
			else if (strcmp(cc.argv[nextarg],"show endian") == 0) {
				cdtprintf (
						 "%s\n"
						 "%d^done\n"
						 "(gdb)\n",
						 "~\"The target endianness is set automatically (currently little endian)\\n\"",
						 cc.sequence);
			}
			else if (strcmp(cc.argv[nextarg],"p/x (char)-1") == 0) {
				cdtprintf (
						 "%s\n"
						 "%d^done\n"
						 "(gdb)\n",
						 "~\"$1 = 0xff\\n\"",
						 cc.sequence);
			}
			else if (strcmp(cc.argv[nextarg],"kill") == 0) {
				if (process.IsValid()) {
					logprintf (LOG_INFO, "console kill: send SIGINT\n");
					if (process.IsValid()) {
						int state = process.GetState ();
						if (state == eStateStopped) {			// if process is stopped. restart it before kill
							SBThread thread = process.GetSelectedThread();
							cdtprintf ("%d^running\n", cc.sequence);
							cdtprintf ("*running,thread-id=\"%d\"\n(gdb)\n", thread.GetIndexID());
							process.Continue();
							pstate->isrunning = true;
						}
					}
					pstate->process.Signal(SIGINT);
					cdtprintf (	"%d^done\n(gdb)\n",	cc.sequence);
				}
				else
					cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "The program is not being run.");
			}
			else
				cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "Command unimplemented.");
		}
	}
	// BEAKPOINT COMMANDS
	else if (strcmp(cc.argv[0],"-break-insert")==0) {
		// break-insert --thread-group i1 -f /Users/didier/Projets/LLDB/hello/Sources/hello.c:17
		// break-insert --thread-group i1 -t -f main
		int isoneshot=0;
		char path[LINE_MAX];
		for (; nextarg<cc.argc; nextarg++) {
			if (strcmp(cc.argv[nextarg],"-t")==0)
				isoneshot = 1;
			else if (strcmp(cc.argv[nextarg],"-f")==0)
				strlcpy (path, cc.argv[++nextarg], sizeof(path));
			else
				strlcpy (path, cc.argv[nextarg], sizeof(path));
		}
		char *pline;
		SBBreakpoint breakpoint;
		if ((pline=strchr(path,':')) != NULL) {
			*pline++ = '\0';
			int iline=0;
			sscanf (pline, "%d", &iline);
			breakpoint = target.BreakpointCreateByLocation (path, iline);
		}
		else		// function
			breakpoint = target.BreakpointCreateByName (path, target.GetExecutable().GetFilename());
		if (breakpoint.IsValid()) {
			if (isoneshot)
				breakpoint.SetOneShot(true);
			char breakpointdesc[LINE_MAX];
			cdtprintf ("%d^done,bkpt=%s\n(gdb)\n", cc.sequence,
					formatBreakpoint (breakpointdesc, sizeof(breakpointdesc), breakpoint, pstate));
		}
		else
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-break-delete")==0) {
		// 11-break-delete 1
		// 11^done
		int bpid=0;
		sscanf (cc.argv[nextarg], "%d", &bpid);
		target.BreakpointDelete(bpid);
		cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-break-enable")==0) {
		// 11-break-enable 1
		// 11^done
		int bpid=0;
		sscanf (cc.argv[nextarg], "%d", &bpid);
		SBBreakpoint breakpoint = target.FindBreakpointByID (bpid);
		breakpoint.SetEnabled(1);
		cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-break-disable")==0) {
		// 11-break-disable 1
		// 11^done
		int bpid=0;
		sscanf (cc.argv[nextarg], "%d", &bpid);
		SBBreakpoint breakpoint = target.FindBreakpointByID (bpid);
		breakpoint.SetEnabled(0);
		cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
	}
	// STACK COMMANDS
	else if (strcmp(cc.argv[0],"-list-thread-groups")==0) {
		// list-thread-groups --available
		//    ^error,msg="Can not fetch data now."
		// list-thread-groups
		//    ^done,groups=[{id="i1",type="process",pid="1186",executable="/Users/didier/Projets/LLDB/hello/Debug/hello"}]
		// list-thread-groups i1
		//    ^done,threads=[{id="1",target-id="Thread 0x1503 of process 1186",frame={level="0",addr="0x0000000100000f46",
		//    func="main",args=[],file="../Sources/hello.c",fullname="/Users/didier/Projets/LLDB/hello/Sources/hello.c",
		//    line="15"},state="stopped"}]
		//		233857.215 <<=  |65^done,threads=[{id="1",target-id="Thread 0x89b52 of process 94273",frame={level="0",addr="0x0000000000000ebf",func="main",args=[{name="argc",value="3"},{name="argv",value="0x00007fff5fbffeb0"},{name="envp",value="0x00007fff5fbffed0"}],file="hello.c",fullname="/Users/didier/Projets/LLDB/hello/Debug/../Sources/hello.c",line="69"},state="stopped"},{id="2",target-id="Thread 0x89b61 of process 94273",frame={level="0",addr="0x000000000001648a",func="__semwait_signal"},state="stopped"}]\n(gdb)\n|
		//		         491,415 47^done,threads=[{id="2",target-id="Thread 0x110f of process 96806",frame={level="0",addr="0x00007fff88a3848a",func="??",args=[]},state="stopped"},{id="1",target-id="Thread 0x1003 of process 96806",frame={level="0",addr="0x0000000100000ebf",func="main",args=[{name="argc",value="1"},{name="argv",value="0x7fff5fbff5c0"},{name="envp",value="0x7fff5fbff5d0"}],file="../Sources/hello.c",fullname="/Users/didier/Projets/LLDB/hello/Sources/hello.c",line="69"},state="stopped"}]

		if (cc.available > 0) {
			cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "Can not fetch data now.");
		}
		else if (cc.argv[nextarg] == NULL) {
			char groupsdesc[LINE_MAX];
			int pid=0;
			int groupdesclength = 0;
			SBFileSpec execspec;
			const char *filename=NULL, *filedir=NULL;
			snprintf (groupsdesc, sizeof(groupsdesc), "id=\"%s\",type=\"process\"", pstate->threadgroup);
			groupdesclength = strlen(groupsdesc);
			if (process.IsValid()) {
				pid=process.GetProcessID();
				snprintf (groupsdesc+groupdesclength, sizeof(groupsdesc)-groupdesclength,
						",pid=\"%d\"", pid);
				groupdesclength = strlen(groupsdesc);
			}
			if (target.IsValid()) {
				execspec = target.GetExecutable();
				filename = execspec.GetFilename();
				filedir = execspec.GetDirectory();
			}
			if (filename!=NULL && filedir!=NULL)
				snprintf (groupsdesc+groupdesclength, sizeof(groupsdesc)-groupdesclength,
						",executable=\"%s/%s\"", filedir, filename);
			cdtprintf ("%d^done,groups=[{%s}]\n(gdb)\n", cc.sequence, groupsdesc);
		}
		else if (strcmp(cc.argv[nextarg],pstate->threadgroup) == 0) {
			char threaddesc[BIG_LINE_MAX];
			formatThreadInfo (threaddesc, sizeof(threaddesc), process, -1);
			if (threaddesc[0] != '\0')
				cdtprintf ("%d^done,threads=[%s]\n(gdb)\n", cc.sequence, threaddesc);
			else
				cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "Can not fetch data now.");
		}
	}
	else if (strcmp(cc.argv[0],"-stack-info-depth")==0) {
		// stack-info-depth --thread 1 11
		// 26^done,depth="1"
		int maxdepth = -1;
		if (cc.argv[nextarg] != NULL)
			if (isdigit(*cc.argv[nextarg]))
				sscanf (cc.argv[nextarg++], "%d", &maxdepth);
		if (process.IsValid()) {
			SBThread thread = process.GetSelectedThread();
			int numframes = getNumFrames (thread);
			cdtprintf ("%d^done,depth=\"%d\"\n(gdb)\n", cc.sequence, numframes);
		}
		else
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-stack-list-frames")==0) {
		// stack-list-frame --thread 1 1 1 (min max)
		int startframe=0, endframe=-1;
		if (cc.argv[nextarg] != NULL)
			if (isdigit(*cc.argv[nextarg]))
				sscanf (cc.argv[nextarg++], "%d", &startframe);
		if (cc.argv[nextarg] != NULL)
			if (isdigit(*cc.argv[nextarg]))
				sscanf (cc.argv[nextarg++], "%d", &endframe);
		SBThread thread = process.GetSelectedThread();
		if (endframe<0)
			endframe = getNumFrames (thread);
		else
			++endframe;
		const char *separator="";
		cdtprintf ("%d^done,stack=[", cc.sequence);
		char framedesc[LINE_MAX];
		for (int iframe=startframe; iframe<endframe; iframe++) {
			SBFrame frame = thread.GetFrameAtIndex(iframe);
			formatFrame (framedesc, sizeof(framedesc), frame, WITH_LEVEL);
			cdtprintf ("%s%s", separator, framedesc);
			separator=",";
		}
		cdtprintf ("]\n(gdb)\n");
	}
	else if (strcmp(cc.argv[0],"-stack-list-arguments")==0) {
		// stack-list-arguments --thread 1 1 (print-values) {1 2 (min max)}
		int printvalues=0, startframe=0, endframe=-1;
		if (cc.argv[nextarg] != NULL)
				if (isdigit(*cc.argv[nextarg]))
					sscanf (cc.argv[nextarg++], "%d", &printvalues);
		if (cc.argv[nextarg] != NULL)
				if (isdigit(*cc.argv[nextarg]))
					sscanf (cc.argv[nextarg++], "%d", &startframe);
		if (cc.argv[nextarg] != NULL)
			if (isdigit(*cc.argv[nextarg]))
				sscanf (cc.argv[nextarg++], "%d", &endframe);
		SBThread thread = process.GetSelectedThread();
		if (endframe<0)
			endframe = getNumFrames (thread);
		else
			++endframe;
		const char *separator="";
		cdtprintf ("%d^done,stack-args=[", cc.sequence);
		char argsdesc[LINE_MAX];
		for (int iframe=startframe; iframe<endframe; iframe++) {
			SBFrame frame = thread.GetFrameAtIndex(iframe);
			formatFrame (argsdesc, sizeof(argsdesc), frame, JUST_LEVEL_AND_ARGS);
			cdtprintf ("%s%s", separator, argsdesc);
			separator=",";
		}
		cdtprintf ("]\n(gdb)\n");
	}
	else if (strcmp(cc.argv[0],"-thread-info")==0) {
		char threaddesc[LINE_MAX];
		int threadindexid = -1;
		if (cc.argv[nextarg] != NULL)
			if (isdigit(*cc.argv[nextarg]))
				sscanf (cc.argv[nextarg++], "%d", &threadindexid);
		formatThreadInfo (threaddesc, sizeof(threaddesc), process, threadindexid);
		if (threaddesc[0] != '\0')
			cdtprintf ("%d^done,threads=[%s]\n(gdb)\n", cc.sequence, threaddesc);
		else
			cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "Can not fetch data now.");
	}
	else if (strcmp(cc.argv[0],"-stack-list-locals")==0) {
		// stack-list-locals --thread 1 --frame 0 1
		// stack-list-locals --thread 2 --frame 0 1
		char printvalues[NAME_MAX];		// 1 or --all-values OR 2 or --simple-values
		printvalues[0] = '\0';
		if (++nextarg<cc.argc)
			strlcpy (printvalues, cc.argv[nextarg], sizeof(printvalues));
		bool isValid = false;
		if (process.IsValid()) {
			SBThread thread = process.GetSelectedThread();
			if (thread.IsValid()) {
				SBFrame frame = thread.GetSelectedFrame();
				if (frame.IsValid()) {
					SBFunction function = frame.GetFunction();
					if (function.IsValid()) {
						isValid = true;
						SBValueList localvars = frame.GetVariables(0,1,0,0);
						char varsdesc[BIG_LINE_MAX];			// may be very big
						formatVariables (varsdesc,sizeof(varsdesc),localvars);
						cdtprintf ("%d^done,locals=[%s]\n(gdb)\n", cc.sequence, varsdesc);
					}
				}
			}
		}
		if (!isValid)
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
	}
	// VARIABLES COMMANDS
	else if (strcmp(cc.argv[0],"-var-create")==0) {
		// var-create --thread 1 --frame 0 - * a
		//     name="var1",numchild="0",value="1",type="int",thread-id="1",has_more="0"
		//     name="var2",numchild="1",value="0x100000f76 \"2\"",type="char *",thread-id="1",has_more=a"0"
		char expression[NAME_MAX];
		const char *sep = "";
		*expression = '\0';
		if (*cc.argv[nextarg]=='-' && *cc.argv[nextarg+1]=='*') {
			nextarg += 2;
			while (nextarg<cc.argc) {
				strlcat (expression, sep, sizeof(expression));
				strlcat (expression, cc.argv[nextarg++], sizeof(expression));
				sep = " ";
			}
			SBThread thread = process.GetSelectedThread();
			SBFrame frame = thread.GetSelectedFrame();
			// Find then Evaluate to avoid recreate variable
			SBValue var = getVariable (frame, expression);
			if (var.IsValid() && var.GetError().Success()) {
				int varnumchildren = var.GetNumChildren();
				SBType vartype = var.GetType();
				SBStream stream;
				var.GetExpressionPath(stream);
				const char *varexpressionpath = stream.GetData();
				char vardesc[NAME_MAX];
				formatValue(vardesc,sizeof(vardesc),var);
				if (vartype.IsReferenceType() && varnumchildren==1)	// correct numchildren and value if reference
					--varnumchildren;
				cdtprintf ("%d^done,name=\"%s\",numchild=\"%d\",value=\"%s\","
							"type=\"%s\",thread-id=\"%d\",has_more=\"0\"\n(gdb)\n",
							cc.sequence, varexpressionpath, varnumchildren, vardesc,
							vartype.GetDisplayTypeName(), thread.GetIndexID());
			}
			else
				cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
		}
		else
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-var-update")==0) {
		// 47-var-update 1 var2
		// 47^done,changelist=[]
		// 41^done,changelist=[{name="var3",value="44",in_scope="true",type_changed="false",has_more="0"}]
		int printvalues=1;
		char expression[NAME_MAX];
		if (cc.argv[nextarg] != NULL)							// print-values
			if (isdigit(*cc.argv[nextarg]))
				sscanf (cc.argv[nextarg++], "%d", &printvalues);
		if (nextarg<cc.argc)									// variable name
			strlcpy (expression, cc.argv[nextarg++], sizeof(expression));
		SBThread thread = process.GetSelectedThread();
		SBFrame frame = thread.GetSelectedFrame();
		SBValue var = getVariable (frame, expression);			// find variable
		char changedesc[BIG_LINE_MAX];
		changedesc[0] = '\0';
		if (var.IsValid() && var.GetError().Success()) {
			bool separatorvisible = false;
			formatChangedList (changedesc, sizeof(changedesc), var, separatorvisible);
		}
		cdtprintf ("%d^done,changelist=[%s]\n(gdb)\n", cc.sequence, changedesc);
	}
	else if (strcmp(cc.argv[0],"-var-list-children")==0) {
		// 34-var-list-children var2
		// 34^done,numchild="1",children=[child={name="var2.*b",exp="*b",numchild="0",type="char",thread-id="1"}],has_more="0"
		// 52-var-list-children var5
		// gdb: 52^done,numchild="3",children=[child={name="var5.a",exp="a",numchild="0",type="int",thread-id="1"},child={name="var5.b",exp="b",numchild="1",type="char *",thread-id="1"},child={name="var5.y",exp="y",numchild="4",type="Y *",thread-id="1"}],has_more="0"
		// mi2: 52^done,numchild="3",children=[child={name="z.a",exp="a",numchild="0",type"int",thread-id="1"},child={name="z.b",exp="b",numchild="1",type"char *",thread-id="1"},child={name="z.y",exp="y",numchild="4",type"Y *",thread-id="1"}]",has_more="0"\n(gdb)\n|
		// 61-var-list-children var5.y
		// 52^done,numchild="3",children=[child={name="var5.a",exp="a",numchild="0",type="int",thread-id="1"},child={name="var5.b",exp="b",numchild="1",type="char *",thread-id="1"},child={name="var5.y",exp="y",numchild="4",type="Y *",thread-id="1"}],has_more="0"
		//|52^done,numchild="3",children=[child={name="z->a",exp="a",numchild="0",type"int",thread-id="1"},child={name="z->b",exp="b",numchild="1",type"char *",thread-id="1"},child={name="z->y",exp="y",numchild="4",type"Y *",thread-id="1"}]",has_more="0"\n(gdb)\n|
		char expression[NAME_MAX];
		*expression = '\0';
		if (nextarg<cc.argc)
			strlcpy (expression, cc.argv[nextarg++], sizeof(expression));
		SBThread thread = process.GetSelectedThread();
		SBFrame frame = thread.GetSelectedFrame();
		SBValue var = getVariable (frame, expression);
		if (var.IsValid() && var.GetError().Success()) {
			int varnumchildren = var.GetNumChildren();
			char childlist[LINE_MAX];
			*childlist = '\0';
			const char *sep="";
			int ichild;
			for (ichild=0; ichild<varnumchildren; ichild++) {
				SBValue child = var.GetChildAtIndex(ichild);
				const char *childname = child.GetName();			// displayed name
				SBStream stream;
				child.GetExpressionPath(stream);					// real path
				const char *childexpressionpath = stream.GetData();
				int childnumchildren = child.GetNumChildren();
				SBType childtype = child.GetType();
				int threadindexid=1;
				if (thread.IsValid())
					threadindexid = thread.GetIndexID();
				// [child={name="var2.*b",exp="*b",numchild="0",type="char",thread-id="1"}]
				int childlistlength = strlen(childlist);
				snprintf (childlist+childlistlength, sizeof(childlist)-childlistlength,
						"%schild={name=\"%s\",exp=\"%s\",numchild=\"%d\","
						"type=\"%s\",thread-id=\"%d\"}",
						sep,childexpressionpath,childname,childnumchildren,childtype.GetDisplayTypeName(),threadindexid);
				sep = ",";
			}
			// 34^done,numchild="1",children=[child={name="var2.*b",exp="*b",numchild="0",type="char",thread-id="1"}],has_more="0"
			cdtprintf ("%d^done,numchild=\"%d\",children=[%s]\",has_more=\"0\"\n(gdb)\n",
					cc.sequence, varnumchildren, childlist);
		}
		else
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
	}
	else if (strcmp(cc.argv[0],"-var-info-path-expression")==0) {
		// 35-var-info-path-expression var2.*b
		// 35^done,path_expr="*(b)"
		// 55-var-info-path-expression var5.y
		// 55^done,path_expr="(z)->y"
		// 65-var-info-path-expression var5.y.d
		// 65^done,path_expr="((z)->y)->d"
		char expression[NAME_MAX];
		*expression = '\0';
		if (nextarg<cc.argc)
			strlcpy (expression, cc.argv[nextarg++], sizeof(expression));
		if (*expression!='$')		// it is yet a path
			cdtprintf ("%d^done,path_expr=\"%s\"\n(gdb)\n", cc.sequence, expression);
		else {
			SBThread thread = process.GetSelectedThread();
			SBFrame frame = thread.GetSelectedFrame();
			SBValue var = getVariable (frame, expression);
			if (var.IsValid() && var.GetError().Success()) {
				SBStream stream;
				var.GetExpressionPath(stream,true);
				const char *varexpressionpath = stream.GetData();
				cdtprintf ("%d^done,path_expr=\"%s\"\n(gdb)\n", cc.sequence, varexpressionpath);
			}
			else
				cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
		}
	}
	else if (strcmp(cc.argv[0],"-var-evaluate-expression")==0 || strcmp(cc.argv[0],"-data-evaluate-expression")==0) {
		// 36-var-evaluate-expression --thread-group i1 "sizeof (void*)"
		// 36^done,value="50 '2'"
		// 58-var-evaluate-expression var5.y
		// 58^done,value="0x100001028 <y>"
		// 66-var-evaluate-expression var5.y.a
		// 66^done,value="0"
		// 36-data-evaluate-expression --thread-group i1 "sizeof (void*)"
		// 36^done,value="8"
		char expression[NAME_MAX];
		if (nextarg<cc.argc)
			strlcpy (expression, cc.argv[nextarg++], sizeof(expression));
		if (strcmp(expression,"sizeof (void*)")==0)
			cdtprintf ("%d^done,value=\"8\"\n(gdb)\n", cc.sequence);
		else {
			SBThread thread = process.GetSelectedThread();
			SBFrame frame = thread.GetSelectedFrame();
			SBValue var = getVariable (frame, expression);		// createVariable
			if (var.IsValid() && var.GetError().Success()) {
				char vardesc[NAME_MAX];
				cdtprintf ("%d^done,value=\"%s\"\n(gdb)\n", cc.sequence, formatValue (vardesc,sizeof(vardesc),var));
			}
			else
				cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
		}
	}
	else if (strcmp(cc.argv[0],"-var-set-format")==0) {
		// 36-var-set-format var3 natural
		// 36-var-set-format var3 binary
		// 36-var-set-format var3 octal
		// 36-var-set-format var3 decimal
		// 36-var-set-format var3 hexadecimal
		// 36^done,format="octal",value="062"
		// 36^done,format="natural",value="0x100000f94 \"22\""
		// 36^done,format="natural",value="50 '2'"
		char expression[NAME_MAX], format[NAME_MAX];
		if (nextarg<cc.argc)
			strlcpy (expression, cc.argv[nextarg++], sizeof(expression));
		if (nextarg<cc.argc)
			strlcpy (format, cc.argv[nextarg++], sizeof(format));
		Format formatcode;
		if (strcmp(format,"binary")==0)
			formatcode = eFormatBinary;
		else if (strcmp(format,"octal")==0)
			formatcode = eFormatOctal;
		else if (strcmp(format,"decimal")==0)
			formatcode = eFormatDecimal;
		else if (strcmp(format,"hexadecimal")==0)
			formatcode = eFormatHex;
		else
			formatcode = eFormatDefault;
		SBThread thread = process.GetSelectedThread();
		SBFrame frame = thread.GetSelectedFrame();
		SBValue var = getVariable (frame, expression);
		if (var.IsValid() && var.GetError().Success()) {
			var.SetFormat(formatcode);
			char vardesc[NAME_MAX];
			cdtprintf ("%d^done,format=\"%s\",value=\"%s\"\n(gdb)\n",
					cc.sequence, format, formatValue(vardesc,sizeof(vardesc),var));
		}
		else
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
	}
	// OTHER COMMANDS
	else if (strcmp(cc.argv[0],"info")==0) {
		// 96info sharedlibrary
		if (nextarg>=cc.argc)
			cdtprintf ("%d^error\n(gdb)\n", cc.sequence);
		else if (strcmp(cc.argv[nextarg],"sharedlibrary") == 0) {
			// code from lldm-mi
			int nLibraries = 0;
			int nModules = target.GetNumModules();
		    for (int iModule=0; iModule<nModules; iModule++) {
		        SBModule module = target.GetModuleAtIndex(iModule);
		        if (module.IsValid()) {
		            const char *moduleFilePath = module.GetFileSpec().GetDirectory();
		            const char *moduleFileName = module.GetFileSpec().GetFilename();
		            const char *moduleHasSymbols = (module.GetNumSymbols() > 0) ? "Yes" : "No";
		            addr_t addrLoadS = 0xffffffffffffffff;
		            addr_t addrLoadSize = 0;
		            bool bHaveAddrLoad = false;
		            const int nSections = module.GetNumSections();
		            for (int iSection = 0; iSection < nSections; iSection++)
		            {
		                SBSection section = module.GetSectionAtIndex(iSection);
		                addr_t addrLoad = section.GetLoadAddress(target);
		                if (addrLoad != (addr_t) -1) {
		                    if (!bHaveAddrLoad) {
		                        bHaveAddrLoad = true;
		                        addrLoadS = addrLoad;
		                    }
		                    addrLoadSize += section.GetByteSize();
		                }
		            }
					cdtprintf ("~\"0x%016x\t0x%016x\t%s\t\t%s/%s\"\n",
							addrLoadS, addrLoadS + addrLoadSize,
							moduleHasSymbols, moduleFilePath, moduleFileName);
					++nLibraries;
		        }
		    }
		    if (nLibraries==0)
				cdtprintf ("%s\n", "~\"No shared libraries loaded at this time.\n\"");
	    	cdtprintf ("%d^done\n(gdb)\n", cc.sequence);
		}
		else
			cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "Command unimplemented.");
	}
	else if (strcmp(cc.argv[0],"-data-list-register-names")==0) {
		// 95-data-list-register-names --thread-group i1
		// not implemented. no use for now
		cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "Command unimplemented.");
	}
	else {
		logprintf (LOG_WARN, "command not understood: ");
		logdata (LOG_NOHEADER, cc.argv[0], strlen(cc.argv[0]));
		cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc.sequence, "Command unimplemented.");
	}

	return dataflag;
}


// decode command line and fill the cc CDT_COMMAND structure
//   get sequence number
//   convert arguments line in a argv vector
//   decode optional (--option) arguments
int
evalCDTLine (STATE *pstate, const char *cdtline, CDT_COMMAND *cc)
{
	cc->sequence = 0;
	cc->argc = 0;
	cc->arguments[0] = '\0';
	if (cdtline[0] == '\0')	// just ENTER
		return 0;
	int fields = sscanf (cdtline, "%d%[^\0]", &cc->sequence, cc->arguments);
	if (fields < 2) {
		logprintf (LOG_WARN, "invalid command format: ");
		logdata (LOG_NOHEADER, cdtline, strlen(cdtline));
		cdtprintf ("%d^error,msg=\"%s\"\n(gdb)\n", cc->sequence, "invalid command format.");
		return 0;
	}

	cc->threadgroup[0] = '\0';
	cc->thread = cc->frame = cc->available = cc->all = -1;

	fields = scanArgs (cc);

	int field;
	for (field=1; field<fields; field++) {				// arg 0 is the command
		if (strcmp(cc->argv[field],"--thread-group") == 0) {
			strlcpy (cc->threadgroup, cc->argv[++field], sizeof(cc->threadgroup));
			strlcpy (pstate->threadgroup, cc->threadgroup, sizeof(pstate));
		}
		else if (strcmp(cc->argv[field],"--thread") == 0) {
			int actual_thread = cc->thread;
			sscanf (cc->argv[++field], "%d", &cc->thread);
			if (cc->thread!=actual_thread && cc->thread>=0)
				pstate->process.SetSelectedThreadByIndexID (cc->thread);
		}
		else if (strcmp(cc->argv[field],"--frame") == 0) {
			int actual_frame = cc->frame;
			sscanf (cc->argv[++field], "%d", &cc->frame);
			if (cc->frame!=actual_frame && cc->frame>=0) {
				SBThread thread = pstate->process.GetSelectedThread();
				thread.SetSelectedFrame (cc->frame);
			}
		}
		else if (strcmp(cc->argv[field],"--available") == 0)
			cc->available = 1;
		else if (strcmp(cc->argv[field],"--all") == 0)
			cc->all = 1;
		else if (strncmp(cc->argv[field],"--",2) == 0) {
			logprintf (LOG_WARN, "unexpected qualifier %s\n", cc->argv[field]);
			break;
		}
		else
			break;
	}

	return field;
}


// convert argument line in a argv vector
// take care of "
int
scanArgs (CDT_COMMAND *cc)
{
	cc->argc=0;
	char *pa=cc->arguments, *ps;
	while (*pa) {
		if (cc->argc>=MAX_ARGS-2) {		// keep place for final NULL
			logprintf (LOG_ERROR, "arguments table too small (%d)\n", MAX_ARGS);
			break;
		}
		while (isspace(*pa))
			++pa;
		if (*pa=='"') {
			ps = ++pa;
			while (*pa && *pa!='"' && *(pa-1)!='\\')
				++pa;
			if (*pa == '"')
				*pa++ = '\0';
		}
		else {
			ps = pa;
			while (*pa && !isspace(*pa))
				++pa;
			if (isspace(*pa))
				*pa++ = '\0';
		}
		cc->argv[cc->argc++] = ps;
	}
	cc->argv[cc->argc] = NULL;
	return cc->argc;
}
