
#include <chrono>
#include <thread>
#include "lldb/API/SBUnixSignals.h"

#include "lldbmi2.h"
#include "log.h"
#include "events.h"
#include "frames.h"


extern LIMITS limits;

static pthread_t sbTID;


void
setSignals (STATE *pstate)
{
	logprintf (LOG_TRACE, "setSignals (0x%x)\n", pstate);
	SBUnixSignals us = pstate->process.GetUnixSignals();
	if (!limits.istest || true) {
		const char *signame = "SIGINT";
		int signo = us.GetSignalNumberFromName(signame);
		logprintf (LOG_NONE, "signals before for %s (%d): suppress=%d, stop=%d, notify=%d\n",
				signame, signo, us.GetShouldSuppress(signo), us.GetShouldStop(signo), us.GetShouldNotify(signo));
		us.SetShouldSuppress (signo,false);			// !pass
		us.SetShouldStop (signo,false);
		us.SetShouldNotify (signo,true);
		logprintf (LOG_NONE, "signals after for %s (%d): suppress=%d, stop=%d, notify=%d\n",
				signame, signo, us.GetShouldSuppress(signo), us.GetShouldStop(signo), us.GetShouldNotify(signo));
	}
}

void
terminateProcess (STATE *pstate, int how)
{
	logprintf (LOG_TRACE, "terminateProcess (0x%x, 0x%x)\n", pstate, how);
	if (pstate->process.IsValid()) {
		SBThread thread = pstate->process.GetSelectedThread();
		int tid = thread.IsValid()? thread.GetIndexID():0;
		if ((how&PRINT_THREAD))
			cdtprintf ("=thread-exited,id=\"%d\",group-id=\"%s\"\n", tid, pstate->threadgroup);
		pstate->process.Destroy();
	//	pstate->process.Kill();
	}
	else
		logprintf (LOG_INFO, "pstate->process not valid\n");
	if ((how&PRINT_GROUP))
		cdtprintf ("=thread-group-exited,id=\"%s\",exit-code=\"0\"\n", pstate->threadgroup);
	if ((how&AND_EXIT))
		pstate->eof = true;
}


int
startProcessListener (STATE *pstate)
{
	logprintf (LOG_TRACE, "startProcessListener (0x%x)\n", pstate);
	int ret = pthread_create (&sbTID, NULL, &processListener, pstate);
	if (ret)
		sbTID = 0;
	return ret;
}

void
waitProcessListener ()
{
	logprintf (LOG_TRACE, "waitProcessListener ()\n");
	if (sbTID)
		pthread_join (sbTID, NULL);
}

// wait thread
void *
processListener (void *arg)
{
	logprintf (LOG_TRACE, "processListener (0x%x)\n", arg);
	STATE *pstate = (STATE *) arg;
	SBProcess process = pstate->process;

 	if (!process.IsValid())
		return NULL;

	if (!pstate->listener.IsValid())
		return NULL;

	while (!pstate->eof) {
		SBEvent event;
		bool gotevent = pstate->listener.WaitForEvent (1000, event);
		if (!gotevent || !event.IsValid())
			continue;
		uint32_t eventtype = event.GetType();
		if (SBProcess::EventIsProcessEvent(event)) {
			StateType processstate = process.GetState();
			switch (eventtype) {
			case SBProcess::eBroadcastBitStateChanged:
				logprintf (LOG_EVENTS|LOG_RAW, "eBroadcastBitStateChanged\n");
				switch (processstate) {
				case eStateRunning:
					logprintf (LOG_EVENTS, "eStateRunning\n");
					break;
				case eStateExited:
					logprintf (LOG_EVENTS, "eStateExited\n");
					checkThreadsLife (pstate, process);		// not useful. threads are not stopped before exit
					terminateProcess (pstate, PRINT_GROUP|AND_EXIT);
					cdtprintf ("*stopped,reason=\"exited-normally\"\n(gdb)\n");
					logprintf (LOG_INFO, "processlistener. eof=%d\n", pstate->eof);
					break;
				case eStateStopped:
					logprintf (LOG_EVENTS, "eStateStopped\n");
					onStopped (pstate, process);
					break;
				default:
					logprintf (LOG_WARN, "unexpected process state %d\n", processstate);
					break;
				}
				break;
			case SBProcess::eBroadcastBitInterrupt:
				logprintf (LOG_EVENTS, "eBroadcastBitInterrupt\n");
				break;
			case SBProcess::eBroadcastBitProfileData:
				logprintf (LOG_EVENTS, "eBroadcastBitProfileData\n");
				break;
			case SBProcess::eBroadcastBitSTDOUT:
			case SBProcess::eBroadcastBitSTDERR:
				// pass stdout and stderr from application to pty
				long iobytes;
				char iobuffer[LINE_MAX];
				if (eventtype==SBProcess::eBroadcastBitSTDOUT)
					logprintf (LOG_EVENTS, "eBroadcastBitSTDOUT\n");
				else
					logprintf (LOG_EVENTS, "eBroadcastBitSTDERR\n");
				iobytes = process.GetSTDOUT (iobuffer, sizeof(iobuffer));
				if (iobytes > 0) {
					// remove \r
					char *ps=iobuffer, *pd=iobuffer;
					do {
						if (*ps=='\r' && *(ps+1)=='\n') {
							++ps;
							--iobytes;
						}
						*pd++ = *ps++;
					} while (*(ps-1));
					write ((pstate->ptyfd!=EOF)?pstate->ptyfd:STDOUT_FILENO, iobuffer, iobytes);
				}
				logdata (LOG_PROG_IN, iobuffer, iobytes);
				break;
			default:
				logprintf (LOG_WARN, "unknown event type %s\n", eventtype);
				break;
			}
		}
		else
			logprintf (LOG_EVENTS, "event type %s\n", eventtype);
	}
	logprintf (LOG_EVENTS, "processlistener exited. pstate->eof=%d\n", pstate->eof);
	return NULL;
}


void
onStopped (STATE *pstate, SBProcess process)
{
	logprintf (LOG_TRACE, "onStopped (0x%x, 0x%x)\n", pstate, &process);
//	-3-38-5.140 <<=  |=breakpoint-modified,bkpt={number="breakpoint 1",type="breakpoint",disp="del",enabled="y",addr="0x0000000100000f06",func="main",file="tests.c",fullname="tests.c",line="33",thread-groups=["i1"],times="1",original-location="tests.c:33"}\n|
//	-3-38-5.140 <<=  |*stopped,reason="breakpoint-hit",disp="keep",bkptno="breakpoint 1",frame={addr="0x0000000100000f06",func="main",args=[],file="tests.c",fullname="tests.c",line="33"},thread-id="1",stopped-threads="all"\n|
//	-3-40-7.049 <<=  |*stopped,reason="breakpoint-hit",disp="keep",bkptno="1",frame={addr="0000000000000f06",func="main",args=[],file="tests.c",fullname="/project_path/tests/Debug/../Sources/tests.c",line="33"},thread-id="1",stopped-threads="all"(gdb)\n|
//                    *stopped,reason="signal-received",signal-name="SIGSEGV",signal-meaning="Segmentation fault",frame={addr="0x0000000100000f7b",func="main",args=[],file="../Sources/tests.cpp",fullname="/project_path/test_hello_cpp/Sources/tests.cpp",line="44"},thread-id="1",stopped-threads="all"
	checkThreadsLife (pstate, process);
	updateSelectedThread (process);				// search which thread is stopped
	SBTarget target = process.GetTarget();
	SBThread thread = process.GetSelectedThread();
    if (!thread.IsValid()) {
    	logprintf (LOG_ERROR, "thread invalid on event eStateStopped\n");
    	return;
    }
	int stopreason = thread.GetStopReason();
//	logprintf (LOG_EVENTS, "stopreason=%d\n", stopreason);
	if (stopreason==eStopReasonBreakpoint || stopreason==eStopReasonPlanComplete) {
		int bpid=0;
		const char *dispose = "keep";
		char reasondesc[LINE_MAX];
		if (stopreason==eStopReasonBreakpoint) {
			if (thread.GetStopReasonDataCount() > 0) {
				int bpid = thread.GetStopReasonDataAtIndex(0);
				SBBreakpoint breakpoint = target.FindBreakpointByID (bpid);
				if (breakpoint.IsOneShot())
					dispose = "del";
				char breakpointdesc[LINE_MAX];
				formatBreakpoint (breakpointdesc, sizeof(breakpointdesc), breakpoint, pstate);
				cdtprintf ("=breakpoint-modified,bkpt=%s\n", breakpointdesc);
				snprintf (reasondesc, sizeof(reasondesc), "reason=\"breakpoint-hit\",disp=\"%s\",bkptno=\"%d\",", dispose, bpid);
			}
			else
				snprintf (reasondesc, sizeof(reasondesc), "reason=\"function-finished\",");
		}
		else
			reasondesc[0] = '\0';
		SBFrame frame = thread.GetSelectedFrame();
		if (!frame.IsValid()) {
	    	logprintf (LOG_ERROR, "frame invalid on event eStateStopped (eStopReasonBreakpoint)\n");
	    	return;
		}
		char framedesc[LINE_MAX];
		formatFrame (framedesc, sizeof(framedesc), frame, WITH_ARGS);
		int threadindexid=thread.GetIndexID();
		cdtprintf ("*stopped,%s%s,thread-id=\"%d\",stopped-threads=\"all\"\n(gdb)\n",
					reasondesc,framedesc,threadindexid);
	//	cdtprintf ("*stopped,reason=\"breakpoint-hit\",disp=\"keep\",bkptno=\"1\",frame={addr=\"0x0000000100000f06\",func=\"main\",args=[],file=\"../Sources/tests.c\",fullname=\"/project_path/tests/Sources/tests.c\",line=\"33\"},thread-id=\"1\",stopped-threads=\"all\"\n(gdb) \n");
		if (strcmp(dispose,"del")==0) {
			target.BreakpointDelete(bpid);
			cdtprintf ("=breakpoint-deleted,id=\"%d\"\n", bpid);
		}
	}
	else if (stopreason==eStopReasonSignal) {
		// raised when attaching to a process
		// raised when program crashed
		char reasondesc[LINE_MAX];
		int stopreason = thread.GetStopReasonDataAtIndex(0);
	 	SBUnixSignals unixsignals = process.GetUnixSignals();
		const char *signalname = unixsignals.GetSignalAsCString(stopreason);
		snprintf (reasondesc, sizeof(reasondesc), "reason=\"signal-received\",signal-name=\"%s\",", signalname);
		SBFrame frame = thread.GetSelectedFrame();
		if (!frame.IsValid()) {
	    	logprintf (LOG_ERROR, "frame invalid on event eStateStopped (eStopReasonSignal)\n");
	    	return;
		}
		char framedesc[LINE_MAX];
		formatFrame (framedesc, sizeof(framedesc), frame, WITH_ARGS);
		int threadindexid = thread.GetIndexID();
		//signal-name="SIGSEGV",signal-meaning="Segmentation fault"
		cdtprintf ("*stopped,%s%sthread-id=\"%d\",stopped-threads=\"all\"\n(gdb)\n",
					reasondesc,framedesc,threadindexid);
		// *stopped,reason="signal-received",signal-name="SIGSEGV",signal-meaning="Segmentation fault",frame={addr="0x0000000100000f7b",func="main",args=[],file="../Sources/tests.cpp",fullname="/project_path/test_hello_cpp/Sources/tests.cpp",line="44"},thread-id="1",stopped-threads="all"
	}
	else if (stopreason==eStopReasonNone) {
		// raised when a thread different from the selected thread stops
	}
	else if (stopreason==eStopReasonInvalid) {
		// raised when the program exits
	}
	else if (stopreason==eStopReasonException) {
		//  "*stopped,reason=\"exception-received\",exception=\"%s\",thread-id=\"%d\",stopped-threads=\"all\""
		char exceptiondesc[LINE_MAX];
		thread.GetStopDescription(exceptiondesc,LINE_MAX);
		write ((pstate->ptyfd!=EOF)?pstate->ptyfd:STDOUT_FILENO, exceptiondesc, strlen(exceptiondesc));
		write ((pstate->ptyfd!=EOF)?pstate->ptyfd:STDOUT_FILENO, "\n", 1);
		char reasondesc[LINE_MAX];
		snprintf (reasondesc, sizeof(reasondesc), "reason=\"exception-received\",exception=\"%s\",", exceptiondesc);
		int threadindexid = thread.GetIndexID();
		cdtprintf ("*stopped,%sthread-id=\"%d\",stopped-threads=\"all\"\n(gdb)\n",
					reasondesc,threadindexid);
	}
	else
		logprintf (LOG_WARN, "unexpected stop reason %d\n", stopreason);
	pstate->isrunning = false;
}


void
checkThreadsLife (STATE *pstate, SBProcess process)
{
	logprintf (LOG_TRACE, "checkThreadsLife (0x%x, 0x%x)\n", pstate, &process);
	if (!process.IsValid())
		return;
	SBThread thread;
	const size_t nthreads = process.GetNumThreads();
	int indexlist;
	bool stillalive[THREADS_MAX];
	for (indexlist=0; indexlist<THREADS_MAX; indexlist++)			// init live list
		stillalive[indexlist] = false;
	for (size_t indexthread=0; indexthread<nthreads; indexthread++) {
		SBThread thread = process.GetThreadAtIndex(indexthread);
		if (thread.IsValid()) {
			int stopreason = thread.GetStopReason();
			int threadindexid = thread.GetIndexID();
			logprintf (LOG_NONE, "thread threadindexid=%d stopreason=%d\n", threadindexid, stopreason);
			for (indexlist=0; indexlist<THREADS_MAX; indexlist++) {
				if (threadindexid == pstate->threadids[indexlist])	// existing thread
					break;
			}
			if (indexlist<THREADS_MAX)								// existing thread. mark as alive
				stillalive[indexlist] = true;
			else {													// new thread. add to the thread list list
				for (indexlist=0; indexlist<THREADS_MAX; indexlist++) {
					if (pstate->threadids[indexlist]==0) {
						pstate->threadids[indexlist] = threadindexid;
						stillalive[indexlist] = true;
						cdtprintf ("=thread-created,id=\"%d\",group-id=\"%s\"\n", threadindexid, pstate->threadgroup);
						break;
					}
				}
				if (indexlist >= THREADS_MAX)
					logprintf (LOG_ERROR, "threads table too small (%d)\n", THREADS_MAX);
			}
		}
	}
	for (indexlist=0; indexlist<THREADS_MAX; indexlist++) {			// find finished threads
		if (pstate->threadids[indexlist]>0 && !stillalive[indexlist]) {
			cdtprintf ("=thread-exited,id=\"%d\",group-id=\"%s\"\n",
					pstate->threadids[indexlist], pstate->threadgroup);
			pstate->threadids[indexlist] = 0;
		}
	}
}


void
updateSelectedThread (SBProcess process)
{
	logprintf (LOG_TRACE, "updateSelectedThread (0x%x)\n", &process);
    if (!process.IsValid())
        return;
    SBThread currentThread = process.GetSelectedThread();
    const StopReason eCurrentThreadStoppedReason = currentThread.GetStopReason();
    if (!currentThread.IsValid() || (eCurrentThreadStoppedReason == eStopReasonInvalid) ||
    		(eCurrentThreadStoppedReason == eStopReasonNone)) {
        // Prefer a thread that has just completed its plan over another thread as current thread
        SBThread planThread;
        SBThread otherThread;
        const size_t nthreads = process.GetNumThreads();
        for (size_t indexthread=0; indexthread<nthreads; indexthread++) {
            //  GetThreadAtIndex() uses a base 0 index
            //  GetThreadByIndexID() uses a base 1 index
            SBThread thread = process.GetThreadAtIndex(indexthread);
            if (!thread.IsValid()) {
               	logprintf (LOG_ERROR, "thread invalid in updateSelectedThread\n");
                return;
            }
            const StopReason eThreadStopReason = thread.GetStopReason();
            switch (eThreadStopReason) {
                case eStopReasonTrace:
                case eStopReasonBreakpoint:
                case eStopReasonWatchpoint:
                case eStopReasonSignal:
                case eStopReasonException:
                    if (!otherThread.IsValid())
                        otherThread = thread;
                    break;
                case eStopReasonPlanComplete:
                    if (!planThread.IsValid())
                        planThread = thread;
                    break;
                case eStopReasonInvalid:
                case eStopReasonNone:
                default:
                    break;
            }
        }
        if (planThread.IsValid())
            process.SetSelectedThread(planThread);
        else if (otherThread.IsValid())
            process.SetSelectedThread(otherThread);
        else {
        	SBThread thread;
            if (currentThread.IsValid())
                thread = currentThread;
            else
                thread = process.GetThreadAtIndex(0);
            if (thread.IsValid())
                process.SetSelectedThread(thread);
        }
    }
}
