
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
#include "log.h"
#include "version.h"


extern const char *testcommands[];

void help (STATE *pstate)
{
	fprintf (stderr, "%s", pstate->lldbmi2Prompt);
	fprintf (stderr, "Description:\n");
	fprintf (stderr, "   A MI2 interface to LLDB\n");
	fprintf (stderr, "Syntax:\n");
	fprintf (stderr, "   lldbmi2 --version [options]\n");
	fprintf (stderr, "   lldbmi2 --interpreter mi2 [options]\n");
	fprintf (stderr, "   lldbmi2 --test [options]\n");
	fprintf (stderr, "Arguments:\n");
	fprintf (stderr, "   --version:     Return GDB's version (GDB 7.7.1) and exits.\n");
	fprintf (stderr, "   --interpreter: Standard mi2 interface.\n");
	fprintf (stderr, "Options:\n");
	fprintf (stderr, "   --log:         Create log file in project root directory.\n");
	fprintf (stderr, "   --logmask:     Select log categories. 0xFFF. See source code for values.\n");
	fprintf (stderr, "   --test:        Execute test sequence (to debug lldmi2).\n");
	fprintf (stderr, "   --nx:          Ignored.\n");
}


static STATE state;
bool global_istest = false;


int
main (int argc, char **argv, char **envp)
{
	int narg;
	fd_set set;
	char line[LINE_MAX];
	long chars;
	struct timeval timeout;
	int isVersion=0, isInterpreter=0;
	const char *pTestCommand=NULL;
	int  idTestCommand=0;
	int  isLog=0;
	int  logmask=LOG_ALL;

	memset (&state, '\0', sizeof(state));
	state.ptyfd = EOF;
	state.gdbPrompt = "GNU gdb (GDB) 7.7.1\n";
	sprintf (state.lldbmi2Prompt, "lldbmi2 version %s, Copyright (C) 2015 Didier Bertrand\n", LLDBMI2_VERSION);

	state.logbuffer[0] = '\0';

	// get args
	for (narg=0; narg<argc; narg++) {
		strlcat (state.logbuffer, argv[narg], sizeof(state.logbuffer));
		strlcat (state.logbuffer, " ", sizeof(state.logbuffer));
		if (strcmp (argv[narg],"--version") == 0)
			isVersion = 1;
		else if (strcmp (argv[narg],"--interpreter") == 0 ) {
			isInterpreter = 1;
			if (++narg<argc) {
				strlcat (state.logbuffer, argv[narg], sizeof(state.logbuffer));
				strlcat (state.logbuffer, " ", sizeof(state.logbuffer));
			}
		}
		else if (strcmp (argv[narg],"--test") == 0 ) {
			state.istest = true;
			global_istest = true;
		}
		else if (strcmp (argv[narg],"--log") == 0 )
			isLog = 1;
		else if (strcmp (argv[narg],"--logmask") == 0 ) {
			isLog = 1;
			if (++narg<argc)
				sscanf (argv[narg], "%x", &logmask);
		}
	}

	// create a log filename from program name and open log file
	if (isLog) {
		if (state.istest)
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
	for (int ienv=0; envp[ienv]; ienv++)
		addEnvironment (&state, envp[ienv]);

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
		if (state.istest)
			logprintf (LOG_NONE, "main loop\n");
		// get command from CDT
		timeout.tv_sec  = 0;
		timeout.tv_usec = 200000;
		FD_SET (STDIN_FILENO, &set);
		select(STDIN_FILENO+1, &set, NULL, NULL, &timeout);
		if (FD_ISSET(STDIN_FILENO, &set) && !state.eof && !state.istest) {
			logprintf (LOG_NONE, "read in\n");
			chars = read (STDIN_FILENO, line, sizeof(line)-1);
			logprintf (LOG_NONE, "read out %d\n", chars);
			if (chars>0) {
				line[chars] = '\0';
				while (fromCDT (&state,line,sizeof(line)) == MORE_DATA)
					line[0] = '\0';
			}
			else
				state.eof = true;
		}
		// execute test command if test mode
		if (!state.lockcdt && !state.eof && state.istest && !state.isrunning) {
			if ((pTestCommand=getTestCommand (&idTestCommand))!=NULL) {
				snprintf (line, sizeof(line), "%s\n", pTestCommand);
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

const char *
getTestCommand (int *idTestCommand)
{
	logprintf (LOG_TRACE, "getTestCommand (0x%x)\n", idTestCommand);
	const char *commandLine;
	if (testcommands[*idTestCommand]!=NULL) {
		commandLine = testcommands[*idTestCommand];
		++*idTestCommand;
		write(STDOUT_FILENO, commandLine, strlen(commandLine));
		write(STDOUT_FILENO, "\n", 1);
		return commandLine;
	}
	else
		return NULL;
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
	else
		logprintf (LOG_INFO, "signal %s\n", signo);
	if (signo==SIGINT) {
		if (state.process.IsValid() && signals_received==0) {
			int selfPID = getpid();
			int processPID = state.process.GetProcessID();
			logprintf (LOG_INFO, "signal_handler: signal SIGINT. self PID = %d, process pid = %d\n", selfPID, processPID);
			logprintf (LOG_INFO, "send signal SIGINT to process %d\n", processPID);
			state.process.Signal (SIGINT);
			++signals_received;
		}
		else
			state.debugger.DispatchInputInterrupt();
	}
}
