
#ifndef FORMAT_H
#define FORMAT_H

typedef enum
{
	WITH_LEVEL			= 0x1,
	WITH_ARGS			= 0x2,
	WITH_LEVEL_AND_ARGS	= 0x3,
	JUST_LEVEL_AND_ARGS	= 0x7
} FrameDetails;


char *  formatBreakpoint (char *breakpointdesc, int descsize, SBBreakpoint breakpoint, STATE *pstate);
int     getNumFrames (SBThread thread);
char *  formatFrame (char *framedesc, int descsize, SBFrame frame, FrameDetails details);
char *  formatThreadInfo (char *threaddesc, int descsize, SBProcess process, int threadindexid);
char *  formatChangedList (char *changedesc, int descsize, SBValue var, bool &separatorvisible);
int     countChangedList (SBValue var);
SBValue getVariable (SBFrame frame, const char *expression);
char *  formatVariables (char *varsdesc, int descsize, SBValueList varslist);
char *  formatValue (char *varsdesc, int descsize, SBValue var);

#endif // FORMAT_H
