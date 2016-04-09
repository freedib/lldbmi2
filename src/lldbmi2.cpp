
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <util.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <stdarg.h>

#include "lldbmi2.h"
#include "engine.h"
#include "variables.h"
#include "log.h"
#include "test.h"
#include "version.h"


void help (STATE *pstate)
{
	fprintf (stderr, "%s", pstate->lldbmi2Prompt);
	fprintf (stderr, "Description:\n");
	fprintf (stderr, "   A MI2 interface to LLDB\n");
	fprintf (stderr, "Author:\n");
	fprintf (stderr, "   Didier Bertrand, 2015\n");
	fprintf (stderr, "Syntax:\n");
	fprintf (stderr, "   lldbmi2 --version [options]\n");
	fprintf (stderr, "   lldbmi2 --interpreter mi2 [options]\n");
	fprintf (stderr, "   lldbmi2 --test [options]\n");
	fprintf (stderr, "Arguments:\n");
	fprintf (stderr, "   --version:           Return GDB's version (GDB 7.7.1) and exits.\n");
	fprintf (stderr, "   --interpreter mi2:   Standard mi2 interface.\n");
	fprintf (stderr, "Options:\n");
	fprintf (stderr, "   --log:                Create log file in project root directory.\n");
	fprintf (stderr, "   --logmask mask:       Select log categories. 0xFFF. See source code for values.\n");
	fprintf (stderr, "   --test n:             Execute test sequence (to debug lldmi2).\n");
	fprintf (stderr, "   --script file_path:   Execute test script or replay logfile (to debug lldmi2).\n");
	fprintf (stderr, "   --nx:                 Ignored.\n");
	fprintf (stderr, "   --frames frames:      Max number of frames to display (%d).\n", FRAMES_MAX);
	fprintf (stderr, "   --children children:  Max number of children to check for update (%d).\n", CHILDREN_MAX);
	fprintf (stderr, "   --walkdepth depth:    Max walk depth in search for variables (%d).\n", WALK_DEPTH_MAX);
	fprintf (stderr, "   --changedepth depth:  Max depth to check for updated variables (%d).\n", CHANGE_DEPTH_MAX);
}


LIMITS limits;
static STATE state;

int
main (int argc, char **argv, char **envp)
{
	int narg;
	fd_set set;
	char line[BIG_LINE_MAX];				// data from cdt
	char consoleLine[LINE_MAX];			// data from eclipse's console
	long chars;
	struct timeval timeout;
	int isVersion=0, isInterpreter=0;
	const char *testCommand=NULL;
	int  isLog=0;
	int  logmask=LOG_ALL;

	memset (&state, '\0', sizeof(state));
	state.ptyfd = EOF;
	state.gdbPrompt = "GNU gdb (GDB) 7.7.1\n";
	sprintf (state.lldbmi2Prompt, "lldbmi2 version %s\n", LLDBMI2_VERSION);

	state.logbuffer[0] = '\0';
	limits.frames_max = FRAMES_MAX;
	limits.children_max = CHILDREN_MAX;
	limits.walk_depth_max = WALK_DEPTH_MAX;
	limits.change_depth_max = CHANGE_DEPTH_MAX;

	// get args
	for (narg=0; narg<argc; narg++) {
		logarg (argv[narg]);
		if (strcmp (argv[narg],"--version") == 0)
			isVersion = 1;
		else if (strcmp (argv[narg],"--interpreter") == 0 ) {
			isInterpreter = 1;
			if (++narg<argc)
				logarg(argv[narg]);
		}
		else if (strcmp (argv[narg],"--test") == 0 ) {
			limits.istest = true;
			if (++narg<argc)
				sscanf (logarg(argv[narg]), "%d", &state.test_sequence);
			if (state.test_sequence)
				setTestSequence (state.test_sequence);
		}
		else if (strcmp (argv[narg],"--script") == 0 ) {
			limits.istest = true;
			if (++narg<argc)
				strcpy (state.test_script, logarg(argv[narg]));		// no spaces allowed in the name
			if (state.test_script[0])
				setTestScript (state.test_script);
		}
		else if (strcmp (argv[narg],"--log") == 0 )
			isLog = 1;
		else if (strcmp (argv[narg],"--logmask") == 0 ) {
			isLog = 1;
			if (++narg<argc)
				sscanf (logarg(argv[narg]), "%x", &logmask);
		}
		else if (strcmp (argv[narg],"--frames") == 0 ) {
			if (++narg<argc)
				sscanf (logarg(argv[narg]), "%d", &limits.frames_max);
		}
		else if (strcmp (argv[narg],"--children") == 0 ) {
			if (++narg<argc)
				sscanf (logarg(argv[narg]), "%d", &limits.children_max);
		}
		else if (strcmp (argv[narg],"--walkdepth") == 0 ) {
			if (++narg<argc)
				sscanf (logarg(argv[narg]), "%d", &limits.walk_depth_max);
		}
		else if (strcmp (argv[narg],"--changedepth") == 0 ) {
			if (++narg<argc)
				sscanf (logarg(argv[narg]), "%d", &limits.change_depth_max);
		}
	}

	// create a log filename from program name and open log file
	if (isLog) {
		if (limits.istest)
			setlogfile (state.logfilename, sizeof(state.logfilename), argv[0], "lldbmi2t.log");
		else
			setlogfile (state.logfilename, sizeof(state.logfilename), argv[0], "lldbmi2.log");
		openlog (state.logbuffer, sizeof(state.logbuffer), state.logfilename);
		setlogmask (logmask);
	}

	// log program args
	logprintf (LOG_ARGS, "%s\n", state.logbuffer);

	state.envp[0] = NULL;
	state.envpentries = 0;
	state.envspointer = state.envs;
	const char *wl = "PWD=";		// want to get eclipse project_loc if any
	int wll = strlen(wl);
	// copy environment for tested program
	for (int ienv=0; envp[ienv]; ienv++) {
		addEnvironment (&state, envp[ienv]);
		if (strncmp(envp[ienv], wl, wll)==0)
			strcpy (state.project_loc, envp[ienv]+wll);
	}

	// return gdb version if --version
	if (isVersion) {
		writetocdt (state.gdbPrompt);
		writetocdt (state.lldbmi2Prompt);
		return EXIT_SUCCESS;
	}
	// check if --interpreter mi2
	else if (!isInterpreter) {
		help (&state);
		return EXIT_FAILURE;
	}

	initializeSB (&state);
	signal (SIGINT, signalHandler);

	cdtprintf ("(gdb)\n");

	// main loop
	FD_ZERO (&set);
	while (!state.eof) {
		if (limits.istest)
			logprintf (LOG_NONE, "main loop\n");
		// get inputs
		timeout.tv_sec  = 0;
		timeout.tv_usec = 200000;
		// check command from CDT
		FD_SET (STDIN_FILENO, &set);
		if (state.ptyfd != EOF) {
			// check data from Eclipse's console
			FD_SET (state.ptyfd, &set);
			select(state.ptyfd+1, &set, NULL, NULL, &timeout);
		}
		else
			select(STDIN_FILENO+1, &set, NULL, NULL, &timeout);
		if (FD_ISSET(STDIN_FILENO, &set) && !state.eof && !limits.istest) {
			logprintf (LOG_NONE, "read in\n");
			chars = read (STDIN_FILENO, line, sizeof(line)-1);
			logprintf (LOG_NONE, "read out %d chars\n", chars);
			if (chars>0) {
				line[chars] = '\0';
				while (fromCDT (&state,line,sizeof(line)) == MORE_DATA)
					line[0] = '\0';
			}
			else
				state.eof = true;
		}
		if (state.ptyfd != EOF) {
			if (FD_ISSET(state.ptyfd, &set) && !state.eof && !limits.istest) {
				chars = read (state.ptyfd, consoleLine, sizeof(consoleLine)-1);
				if (chars>0) {
					logprintf (LOG_PROG_OUT, "pty read %d chars\n", chars);
					consoleLine[chars] = '\0';
					SBProcess process = state.process;
					if (process.IsValid()) {
						process.PutSTDIN (consoleLine, chars);
					}
				}
			}
		}
		// execute test command if test mode
		if (!state.lockcdt && !state.eof && limits.istest && !state.isrunning) {
			if ((testCommand=getTestCommand ())!=NULL) {
				snprintf (line, sizeof(line), "%s\n", testCommand);
				fromCDT (&state, line, sizeof(line));
			}
		}
		// execute stacked commands if many command arrived once
		if (!state.lockcdt && !state.eof && state.cdtbuffer[0]!='\0') {
			line[0] = '\0';
			while (fromCDT (&state, line, sizeof(line)) == MORE_DATA)
				;
		}
	}

	if (state.ptyfd != EOF)
		close (state.ptyfd);
	terminateSB ();

	logprintf (LOG_INFO, "main exit\n");
	closelog ();

	return EXIT_SUCCESS;
}


// log an argument and return the argument
const char *
logarg (const char *arg) {
	strlcat (state.logbuffer, arg, sizeof(state.logbuffer));
	strlcat (state.logbuffer, " ", sizeof(state.logbuffer));
	return arg;
}

void
writetocdt (const char *line)
{
	logprintf (LOG_NONE, "writetocdt (...)\n", line);
	logdata (LOG_CDT_OUT, line, strlen(line));
	write (STDOUT_FILENO, line, strlen(line));
}

void
cdtprintf ( const char *format, ... )
{
	logprintf (LOG_NONE, "cdtprintf (...)\n");
	char buffer[BIG_LINE_MAX];
	va_list args;

	if (format!=NULL) {
		va_start (args, format);
		vsnprintf (buffer, sizeof(buffer), format, args);
		va_end (args);
		writetocdt (buffer);
		if (strlen(buffer) >= sizeof(buffer)-1)
			logprintf (LOG_ERROR, "cdtprintf buffer size (%d) too small\n", sizeof(buffer));
	}
}

static int signals_received=0;

void
signalHandler (int signo)
{
	logprintf (LOG_TRACE, "signalHandler (%d)\n", signo);
	if (signo==SIGINT)
		logprintf (LOG_INFO, "signal SIGINT\n");
	else if (signo==SIGSTOP)
		logprintf (LOG_INFO, "signal SIGSTOP\n");
	else
		logprintf (LOG_INFO, "signal %s\n", signo);
	if (signo==SIGINT) {
		if (state.process.IsValid() && signals_received==0) {
			int selfPID = getpid();
			int processPID = state.process.GetProcessID();
			logprintf (LOG_INFO, "signal_handler: signal SIGINT. self PID = %d, process pid = %d\n", selfPID, processPID);
			logprintf (LOG_INFO, "send signal SIGSTOP to process %d\n", processPID);
			state.process.Signal (SIGSTOP);
			++signals_received;
		}
		else
			state.debugger.DispatchInputInterrupt();
	}
}
