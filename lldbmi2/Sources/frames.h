
#ifndef FRAMES_H
#define FRAMES_H

typedef enum
{
	WITH_LEVEL			= 0x1,
	WITH_ARGS			= 0x2,
	WITH_LEVEL_AND_ARGS	= 0x3,
	JUST_LEVEL_AND_ARGS	= 0x7
} FrameDetails;


char *  formatBreakpoint (char *breakpointdesc, size_t descsize, SBBreakpoint breakpoint, STATE *pstate);
int     getNumFrames     (STATE *pstate, SBThread thread);
char *  formatFrame      (char *framedesc, size_t descsize, SBFrame frame, FrameDetails details, STATE *pstate);
char *  formatThreadInfo (STATE *pstate, char *threaddesc, size_t descsize, SBProcess process, int threadindexid);

#endif // FORMFRAMES_HAT_H
