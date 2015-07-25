
#ifndef VARIABLES_H
#define VARIABLES_H

#include <lldb/API/LLDB.h>
using namespace lldb;

typedef enum
{
	NO_SUMMARY		= 0x1,
	FULL_SUMMARY	= 0x2,
} VariableDetails;


#define DEPTH_MAX  2			// maximum depyh to check variables changed
#define ARRAY_MAX 200			// maximum number of children to display. must be a multiple of 8

#define min(a,b) ((a) < (b) ? (a) : (b))

int     updateVarState (SBValue var, int depth=DEPTH_MAX);
SBValue getVariable (SBFrame frame, const char *expression);

char *  formatChangedList (char *changedesc, size_t descsize, SBValue var, bool &separatorvisible, int depth=DEPTH_MAX);
char *  formatVariables (char *varsdesc, size_t descsize, SBValueList varslist);
char *  formatSummary (char *summarydesc, size_t descsize, SBValue var);
char *  formatValue (char *varsdesc, size_t descsize, SBValue var, VariableDetails details);
char *  formatExpressionPath (char *expressionpathdesc, size_t descsize, SBValue var);

#endif // VARIABLES_H
