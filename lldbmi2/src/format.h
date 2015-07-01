
#ifndef FORMAT_H
#define FORMAT_H

const char *formatbreakpoint (char *breakpointdesc, int descsize, SBBreakpoint breakpoint, STATE *pstate);
int getNumFrames (SBThread thread);
const char *formatframe (char *framedesc, int descsize, SBFrame frame, bool withlevel);
const char *formatvariables (char *varsdesc, int descsize, SBValueList varslist);
SBValue getVariable (SBFrame frame, const char *expression);
const char *formatvalue (char *varsdesc, int descsize, SBValue var);
const char *formatThreadInfo (char *threaddesc, int descsize, SBProcess process, int threadindexid);

#endif // FORMAT_H
