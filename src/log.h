
#ifndef LOG_H
#define LOG_H


#define DETAILED_DEBUG 1

#if DETAILED_DEBUG==1
#define DEBUG_FUNCTION (f) f
#else
#define DEBUG_FUNCTION (f)
#endif


// Log mask bits definitions.
typedef enum
{
	LOG_NONE 		= (0),			// 0x0000	// no log
	LOG_NOHEADER	= (1 << 0),		// 0x0001
	LOG_ERROR		= (1 << 1),		// 0x0002
	LOG_WARN 		= (1 << 2),		// 0x0004
	LOG_INFO		= (1 << 3),		// 0x0008
	LOG_CDT_IN 		= (1 << 4),		// 0x0010
	LOG_CDT_OUT		= (1 << 5),		// 0x0020
	LOG_PROG_IN		= (1 << 6),		// 0x0040
	LOG_PROG_OUT	= (1 << 7),		// 0x0080
	LOG_EVENTS		= (1 << 8),		// 0x0100
	LOG_ARGS		= (1 << 9),		// 0x0200
	LOG_VARS		= (1 << 10),	// 0x0400	// log variables substitutions in test files
	LOG_UNUSED 		= (1 << 11),	// 0x0800	// not used yet
	LOG_STDERR		= (1 << 12),	// 0x1000	// print also to stderr
	LOG_DEBUG 		= (1 << 13),	// 0x2000	// for temporary debugging.
	LOG_TRACE		= (1 << 14),	// 0x4000	// print function calls
	LOG_RAW 		= (1 << 15),	// 0x8000	// print detail LOG_xxx | LOG_RAW
	LOG_ALL			= (0x3FFF),		// all but LOG_TRACE and LOG_RAW
	LOG_DEV			= (0x7FFF)		// all but LOG_RAW
} LogMask;


// Sets the log path name.
// If parent directory is Debug, set the path to the parent directory
// Else sets the path to program directory
void  setlogfile      (char *logfilename, int filenamesize, const char *progname, const char *logname);
int   openlogfile     (const char *logbuffer);
void  closelogfile    ();
void  setlogmask      (unsigned mask);
char *gettimestamp    ();
const char *getheader (unsigned scope);
void  logprintf       (unsigned scope, const char *format, ...);
void  logdata         (unsigned scope, const char *data, int datasize);
void  lognumbers      (unsigned scope, const unsigned long *data, int datasize);
void  addlog          (const char *string);
void  assertStrings   (char *sa, char *sb);		// temp
void  writelog        (int fd, const void *buf, size_t count);

#endif // LOG_H
