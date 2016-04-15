
#ifndef LLDBMIG_H
#define LLDBMIG_H

#include <lldb/API/LLDB.h>
using namespace lldb;

// use system constants
// PATH_MAX = 1024
// LINE_MAX = 2048
// NAME_MAX = 255
#include <sys/syslimits.h>
#include "stringb.h"

#define USE_BUFFERS			// define this if want to use experimental buffers

#define WAIT_DATA  0
#define MORE_DATA  1

#define FALSE 0
#define TRUE  1

#define THREADS_MAX 50
#define FRAMES_MAX  75

#define VALUE_MAX (NAME_MAX<<1)
#define BIG_VALUE_MAX (NAME_MAX<<3)
#define BIG_LINE_MAX (LINE_MAX<<3)

#define ENV_ENTRIES 100

// static context
typedef struct {
	bool istest;
	int frames_max;
	int children_max;
	int walk_depth_max;
	int change_depth_max;
} LIMITS;

//TODO: implement StringB for all big data arrays

// dynamic context
typedef struct {
	int  ptyfd;
	bool eof;
	bool isrunning;
	bool wanttokill;
	int test_sequence;
	char test_script[PATH_MAX];
	const char *envp[ENV_ENTRIES];
	int  envpentries;
	char envs[BIG_LINE_MAX];
	char *envspointer;
	char project_loc[PATH_MAX];
	char cdtbuffer[BIG_LINE_MAX];		// must be the same size as cdtline in fromCDT
	char cdtptyname[NAME_MAX];
	char logfilename[PATH_MAX];
	const char *gdbPrompt;
	char lldbmi2Prompt[NAME_MAX];
	char threadgroup[NAME_MAX];
	SBDebugger debugger;
	SBProcess process;
	SBListener listener;
	int threadids[THREADS_MAX];
} STATE;

const char * logarg (const char *arg);
void         writetocdt    (const char *line);
void         cdtprintf     (const char *format, ... );
void         signalHandler (int vSigno);

#endif	// LLDBMIG_H
