
#ifndef VARIABLES_H
#define VARIABLES_H

#include <lldb/API/LLDB.h>
using namespace lldb;

typedef enum
{
	NO_SUMMARY		= 0x1,
	FULL_SUMMARY	= 0x2,
} VariableDetails;


#define CHANGED_DEPTH_MAX   3		// maximum depth to check variables changed
#define WALK_DEPTH_MAX     10		// maximum depth to walk variables when searching expressions
#define ARRAY_MAX         200		// maximum number of children to display. must be a multiple of 8
#define CHILDREN_MAX      100		// limit of children to examine when walking in them


#define min(a,b) ((a) < (b) ? (a) : (b))

SBValue getVariable (SBFrame frame, const char *expression);
int     updateVarState (SBValue var, int depth=CHANGED_DEPTH_MAX);

char *  formatExpressionPath (char *expressionpathdesc, size_t descsize, SBValue var);
char *  formatChildrenList (char *childrendesc, size_t descsize, SBValue var, char *expression, int threadindexid, int &varnumchildren);
char *  formatChangedList (char *changedesc, size_t descsize, SBValue var, bool &separatorvisible, int depth=CHANGED_DEPTH_MAX);
char *  formatVariables (char *varsdesc, size_t descsize, SBValueList varslist);
char *  formatSummary (char *summarydesc, size_t descsize, SBValue var);
char *  formatValue (char *varsdesc, size_t descsize, SBValue var, VariableDetails details);

#endif // VARIABLES_H
