
#ifndef LLDBMIG_H
#define LLDBMIG_H

#include </usr/include/sys/syslimits.h>
#include <lldb/API/LLDB.h>
using namespace lldb;

// use system constants
// PATH_MAX = 1024
// LINE_MAX = 2048
// NAME_MAX = 255


#define WAIT_DATA  0
#define MORE_DATA  1

#define FALSE 0
#define TRUE  1

#define MAX_THREADS 3


typedef struct {
	int  lockcdt;
	int  ptyfd;
	bool eof;
	bool running;
	char cdtbuffer[LINE_MAX];
	char cdtptyname[NAME_MAX];
	char logfilename[PATH_MAX];
	char logbuffer[LINE_MAX];
	const char *gdbPrompt;
	char threadgroup[NAME_MAX];
	SBDebugger debugger;
	SBProcess process;
	int threadids[MAX_THREADS];
} STATE;


const char *getTestCommand (int *idTestCommand);
void        writetocdt     (const char *line);
void        cdtprintf      ( const char *format, ... );


#endif	// LLDBMIG_H
