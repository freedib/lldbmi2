
#ifndef FORMAT_H
#define FORMAT_H

typedef enum
{
	WITH_LEVEL			= 0x1,
	WITH_ARGS			= 0x2,
	WITH_LEVEL_AND_ARGS	= 0x3,
	JUST_LEVEL_AND_ARGS	= 0x7
} FrameDetails;


const char *formatbreakpoint (char *breakpointdesc, int descsize, SBBreakpoint breakpoint, STATE *pstate);
int getNumFrames (SBThread thread);
const char *formatframe (char *framedesc, int descsize, SBFrame frame, FrameDetails details);
const char *formatvariables (char *varsdesc, int descsize, SBValueList varslist);
SBValue getVariable (SBFrame frame, const char *expression);
const char *formatvalue (char *varsdesc, int descsize, SBValue var);
const char *formatThreadInfo (char *threaddesc, int descsize, SBProcess process, int threadindexid);

#endif // FORMAT_H
