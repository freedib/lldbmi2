
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


extern const char *testcommands[];

void help ()
{
	fprintf (stderr, "Name:\n");
	fprintf (stderr, "   lldbmi2\n");
	fprintf (stderr, "Description:\n");
	fprintf (stderr, "   An MI2 interface to LLDB\n");
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
	int  isTest=0;
	int  idTestCommand=0;
	int  isLog=0;
	int  logmask=LOG_ALL;
	STATE state;

	memset (&state, '\0', sizeof(state));
	state.ptyfd = EOF;
	state.gdbPrompt = "GNU gdb (GDB) 7.7.1\n";
	state.lldbmi2Prompt = "lldbmi2 gateway version 1.0, Copyright (C) 2015 Didier Bertrand\n";

	state.logbuffer[0] = '\0';
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
		else if (strcmp (argv[narg],"--test") == 0 )
			isTest = 1;
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
		setlogfile (state.logfilename, sizeof(state.logfilename), argv[0], "lldbmi2.log");
		openlog (state.logbuffer, sizeof(state.logbuffer), state.logfilename);
		setlogmask (logmask);
	}

	// log program args
	logprintf (LOG_ARGS, "%s\n", state.logbuffer);

	for (int ienv=0; envp[ienv]; ienv++)
		logprintf (LOG_ARGS|LOG_RAW, "envp[%d]=%s\n", ienv, envp[ienv]);

	if (isVersion) {		// if --gdb and real gsb version is 6.3.5, must alse set --gdb6
		writetocdt (state.gdbPrompt);
		writetocdt (state.lldbmi2Prompt);
		return EXIT_SUCCESS;
	}
	else if (!isInterpreter) {
		help ();
		return EXIT_FAILURE;
	}

	initializeSB (&state);

	cdtprintf ("(gdb)\n");

	// main loop
	FD_ZERO (&set);
	while (!state.eof) {
		// get command from CDT
		timeout.tv_sec  = 0;
		timeout.tv_usec = 200000;
		FD_SET (STDIN_FILENO, &set);
		select(STDIN_FILENO+1, &set, NULL, NULL, &timeout);
		if (FD_ISSET(STDIN_FILENO, &set) && !state.eof) {
			chars = read (STDIN_FILENO, line, sizeof(line)-1);
			if (chars>0) {
				line[chars] = '\0';
				while (fromcdt (&state,line,sizeof(line)) == MORE_DATA)
					line[0] = '\0';
			}
			else
				state.eof = true;
		}
		// execute test command if test mode
		if (!state.lockcdt && !state.eof && isTest && !state.running) {
			if ((pTestCommand=getTestCommand (&idTestCommand))!=NULL) {
				snprintf (line, sizeof(line), "%s\n", pTestCommand);
				fromcdt (&state, line, sizeof(line));
			}
		}
		// execute stacked commands if many command arrived once
		if (!state.lockcdt && !state.eof && state.cdtbuffer[0]!='\0') {
			line[0] = '\0';
			while (fromcdt (&state, line, sizeof(line)) == MORE_DATA)
				;
		}
	}

	logprintf (LOG_INFO, "main loop exited\n");
	if (state.ptyfd != EOF)
		close (state.ptyfd);
	terminateSB ();

	usleep(500000);

//	system("stty sane");
	closelog ();

	return EXIT_SUCCESS;
}


const char *
getTestCommand (int *idTestCommand)
{
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
	logdata (LOG_CDT_OUT, line, strlen(line));
	write (STDOUT_FILENO, line, strlen(line));
}

void
cdtprintf ( const char *format, ... )
{
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
