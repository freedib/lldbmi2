
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <stdarg.h>
#include <string.h>
#include <sys/param.h>
#ifdef __APPLE__
#include <util.h>
#include <sys/syslimits.h>
#else
#include <limits.h>
#include <time.h>
#endif

#include "log.h"
#include "stringb.h"

static int     log_fd=-1;
static StringB logbuffer;
static int     log_mask  = LOG_ALL;

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
		snprintf (logfilename, filenamesize, "%s/logs/%s", penv, logname);		// used for lldbmi2.log
	else {
	// getcwd() gives the path of eclipse, so try environment PWD instead
		penv = getenv("CWD");
		if (penv != NULL)
			snprintf (logfilename, filenamesize, "%s/logs/%s", penv, logname);	// used for lldbmi2t.log
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
openlogfile (const char *logfilename)
{
	umask (S_IWGRP | S_IWOTH);
	log_fd = open (logfilename, O_WRONLY|O_CREAT|O_TRUNC, 0664);
	return log_fd;
}

// close the log file
void
closelogfile ()
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
	static char timestring[20];
#ifdef __APPLE__
	struct timeb time_b;
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
#else
	timespec tp;
	clock_gettime (CLOCK_REALTIME, &tp);
	time_t hms = tp.tv_sec;
	hms %= 86400;
	int h = hms/3600;
	int ms = hms%3600;
	int m = ms/60;
	int s = ms%60;
	snprintf (timestring, sizeof(timestring), "%02d%02d%02d.%03d ", h, m, s, (int)(tp.tv_nsec/1000000L));
#endif
	return timestring;
}

// return a header string corresponding to the given scope
const char *
getheader ( unsigned scope )
{
	switch (scope&LOG_DEV) {
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
	case LOG_VARS:
		return "===";
	case LOG_STDERR:
		return "+++";
	case LOG_DEBUG:
		return "$$$";
	case LOG_TRACE:
		return ":::";
	default:
		return "   ";
	}
	return "  ";
}

// log a message for the given scope
// if format is NULL, print log_buffer
void
logprintf ( unsigned scope, const char *format, ... )
{
	va_list args;
	char *ts;
	const char *header;

	if (scope==LOG_NONE)
		return;
	if (log_fd >= 0 && (scope&log_mask)==scope) {	// if logmask don't include LOG_RAW, LOG_XXX|LOG_RAW won't print
		ts = gettimestamp();
		writelog(log_fd, ts, strlen(ts));
		header = getheader(scope);
		writelog(log_fd, header, strlen(header));
		writelog(log_fd, "  ", 2);
		if (format!=NULL) {
			va_start (args, format);
			logbuffer.vosprintf (0, format, args);
			va_end (args);
		}
		writelog(log_fd, logbuffer.c_str(), logbuffer.size());
		if (scope==LOG_ERROR || scope==LOG_STDERR)
			fprintf (stderr, "%s", logbuffer.c_str());
	}
}

// log data for a given scope
// non printable chars are converted to escape chars or hex string
// used to log CDT I/O and program I/O
void
logdata ( unsigned scope, const char *data, int datasize )
{
	if (data == NULL)
		return;
	if (log_fd >= 0 && (scope&log_mask)==scope) {
		logbuffer.clear();
		logbuffer.append("|");
		for (int ii=0; ii<datasize; ii++) {
			if( ((unsigned char)data[ii])>=0x20 && ((unsigned char)data[ii])<127)
				logbuffer.catsprintf("%c",data[ii]);
			else {
				switch (data[ii]) {
				case '\n':
					logbuffer.append("\\n"); break;
				case '\r':
					logbuffer.append("\\r"); break;
				case '\t':
					logbuffer.append("\\t"); break;
				default:
					logbuffer.catsprintf("{%02X}", data[ii]);
					break;
				}
			}
		}
		logbuffer.append("|\n");
		logprintf (scope, NULL);
	}
}

// log data for a given scope
void
lognumbers ( unsigned scope, const unsigned long *data, int datasize )
{
	if (data == NULL)
		return;
	if (log_fd >= 0 && (scope&log_mask)==scope) {
		logbuffer.catsprintf("%016lx: ", (unsigned long)data);		// pointer size = unsigned long size = 8 bytes = 16 chars
		for (int ii=0; ii<datasize; ii++)
			logbuffer.catsprintf("%016lx ", data[ii]);

		logprintf (scope, NULL);
		writelog (log_fd, logbuffer.c_str(), logbuffer.size());
		writelog (log_fd, "\n", 1);
	}
}

// log an argument and return the argument
void
addlog (const char *string) {
	logbuffer.append (string);
}

void
assertStrings (char *sa, char *sb)			// temp
{
	if (strcmp (sa, sb) != 0)
		logprintf (LOG_ERROR, "Assert error:\n%s\n%s\n", sa, sb);
}

// same as write, but avoid ignoring return value warning
void writelog(int fd, const void *buf, size_t count) {
	if (write (fd, buf, count) <=0)
		logprintf (LOG_WARN, "write error\n");
}
