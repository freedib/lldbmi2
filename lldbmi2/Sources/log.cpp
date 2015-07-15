
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
#include <sys/param.h>

#include "log.h"


static int   log_fd=-1;
static char *log_buffer;
static int   buffer_size = 0;
static int   log_mask  = LOG_ALL;


// a log message is written in the log file
// each log message have a scope which is filtered by the scope mask
// the scope mask can be set thru command line


// set the log filename from the program path
// if under eclipse it should be in the root of the project
void
setlogfile (char *logfilename, int filenamesize, const char *progname, const char *logname)
{
	char *penv;
	// first try with eclipse project root path
	penv = getenv("ProjDirPath");
	if (penv != NULL)
		snprintf (logfilename, filenamesize, "%s/%s", penv, logname);
	else {
	// getcwd() gives the path of eclipse, so try environment PWD instead
		penv = getenv("CWD");
		if (penv != NULL)
			snprintf (logfilename, filenamesize, "%s/%s", penv, logname);
		else {
			// if no environment variable, use getcwd()
			char cwd[PATH_MAX];
			if (getcwd(cwd,PATH_MAX) != NULL)
				snprintf (logfilename, filenamesize, "%s/%s", cwd, logname);
			else
				// no path. open where it can
				snprintf (logfilename, filenamesize, "%s", logname);
		}
	}
#if 0
	fprintf (stderr, "%s\n", logfilename);
#endif
}

// open and truncate the log file
int
openlog (char *logbuffer, int buffersize, const char *logfilename)
{
	umask (S_IWGRP | S_IWOTH);
	log_fd = open (logfilename, O_WRONLY|O_CREAT|O_TRUNC, 0664);
	log_buffer = logbuffer;
	buffer_size = buffersize;
	return log_fd;
}

// close the log file
void
closelog ()
{
	if (log_fd >= 0) {
		close (log_fd);
		log_fd = -1;
	}
}

// set the log scope mask. by default LOG_ALL
void
setlogmask (unsigned mask)
{
	log_mask = mask;
}

// get the current time stamp as a string HMS.millis
char *
gettimestamp ()
{
	struct timeb time_b;
	static char timestring[20];
	ftime(&time_b);
	time_t hms = time_b.time;
	hms -= time_b.timezone*60;
	hms += time_b.dstflag*3600;
	hms %= 86400;
	int h = hms/3600;
	int ms = hms%3600;
	int m = ms/60;
	int s = ms%60;
	snprintf (timestring, sizeof(timestring), "%02d%02d%02d.%03d ", h, m, s, time_b.millitm);
	return timestring;
}

// return a header string corresponding to the given scope
const char *
getheader ( unsigned scope )
{
	switch (scope&LOG_ALL) {
	case LOG_NOHEADER:
		return "";
	case LOG_ERROR:
		return "***";
	case LOG_WARN:
		return "!!!";
	case LOG_INFO:
		return "---";
	case LOG_CDT_IN:
		return (scope&LOG_RAW)? ">==": ">>=";
	case LOG_CDT_OUT:
		return "<<=";
	case LOG_PROG_IN:
		return "<<<";
	case LOG_PROG_OUT:
		return ">>>";
	case LOG_EVENTS:
		return "###";
	case LOG_ARGS:
		return "@@@";
	case LOG_DEBUG:
		return "$$$";
	case LOG_STDERR:
		return "+++";
	default:
		return "???";
	}
	return "  ";
}

// log a message for the given scope
void
logprintf ( unsigned scope, const char *format, ... )
{
	va_list args;
	char *ts;
	const char *header;

	if (scope==LOG_NONE)
		return;
	if (log_fd >= 0 && (scope&log_mask)==scope) {
		ts = gettimestamp();
		write(log_fd, ts, strlen(ts));
		header = getheader(scope);
		write(log_fd, header, strlen(header));
		write(log_fd, "  ", 2);
		if (format!=NULL) {
			va_start (args, format);
			vsnprintf (log_buffer, buffer_size, format, args);
			va_end (args);
			write(log_fd, log_buffer, strlen(log_buffer));
			if (scope==LOG_ERROR || scope==LOG_STDERR)
				fprintf (stderr, "%s", log_buffer);
		}
	}
}

// log data for a given scope
// non printable chars are converted to escape chars or hex string
// used to log CDT I/O and program I/O
void
logdata ( unsigned scope, const char *data, int datasize )
{
	bool toosmall = false;
	if (log_fd >= 0 && (scope&log_mask)==scope) {
		log_buffer[0] = '|';
		int ii, jj;
		for (ii=0, jj=1; ii<datasize; ii++) {
			if (jj>buffer_size-2) {
				toosmall = true;
				break;
			}
			if( ((unsigned char)data[ii])>=0x20 && ((unsigned char)data[ii])<127)
				log_buffer[jj++] = data[ii];
			else {
				if (jj>buffer_size-3) {
					toosmall = true;
					break;
				}
				switch (data[ii]) {
				case '\e':
					log_buffer[jj++]='\\'; log_buffer[jj++]='e'; break;
				case '\n':
					log_buffer[jj++]='\\'; log_buffer[jj++]='n'; break;
				case '\r':
					log_buffer[jj++]='\\'; log_buffer[jj++]='r'; break;
				case '\t':
					log_buffer[jj++]='\\'; log_buffer[jj++]='t'; break;
				default:
					if (jj>buffer_size-5) {
						toosmall = true;
						break;
					}
					sprintf( &log_buffer[jj], "{%02X}", data[ii] );
					jj += 4;
					break;
				}
			}
		}
		log_buffer[jj++] = '|';
		log_buffer[jj] = '\0';

		logprintf (scope, NULL);
		write (log_fd, log_buffer, strlen(log_buffer));
		write (log_fd, "\n", 1);
		if (toosmall)
			logprintf (LOG_ERROR, "log buffer too small (%d)\n", buffer_size);
	}
}
