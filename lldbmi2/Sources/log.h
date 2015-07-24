
#ifndef LOG_H
#define LOG_H


// Log mask bits definitions.
typedef enum
{
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
	LOG_DEBUG 		= (1 << 10),	// 0x0400	// for temporary debugging.
	LOG_STDERR		= (1 << 11),	// 0x0800	// print also to stderr
	LOG_RAW 		= (1 << 12),	// 0x1000	// print detail LOG_xxx | LOG_RAW
	LOG_NONE 		= (1 << 13),	// 0x2000	// no log
	LOG_ALL			= (0x0FFF)		// all but LOG_RAW
} LogMask;


// Sets the log path name.
// If parent directory is Debug, set the path to the parent directory
// Else sets the path to program directory
void  setlogfile      (char *logfilename, int filenamesize, const char *progname, const char *logname);
int   openlog         (char *logbuffer, int buffersize, const char *logfilename);
void  closelog        ();
void  setlogmask      (unsigned mask);
char *gettimestamp    ();
const char *getheader (unsigned scope);
void  logprintf       (unsigned scope, const char *format, ...);
void  logdata         (unsigned scope, const char *data, int datasize);
void  lognumbers      (unsigned scope, const unsigned long *data, int datasize);

#endif // LOG_H
