
#ifndef VARIABLES_H
#define VARIABLES_H

#include <lldb/API/LLDB.h>
using namespace lldb;

#define DEPTH_MAX  2			// maximum depyh to check variables changed
#define ARRAY_MAX 200			// maximum number of children to display. must be a multiple of 8

#define TRACKED_VARS_MAX 100	// maximum number of tracked vars

typedef struct
{
	addr_t functionaddress;
	char varname[NAME_MAX];
	SBData data;
	int changes;
} TRACKED_VAR;

#define min(a,b) ((a) < (b) ? (a) : (b))

int     evaluateVarChanged (addr_t functionaddress, SBValue var, int depth=DEPTH_MAX);
SBValue getVariable (SBFrame frame, const char *expression);

char *  formatChangedList (char *changedesc, size_t descsize, addr_t functionaddress, SBValue var, bool &separatorvisible, int depth=DEPTH_MAX);
char *  formatVariables (char *varsdesc, size_t descsize, SBValueList varslist, addr_t functionaddress);
char *  formatValue (char *varsdesc, size_t descsize, SBValue var, int trackedvarchanges=0);
char *  formatExpressionPath (char *expressionpathdesc, size_t descsize, SBValue var);

#endif // VARIABLES_H
