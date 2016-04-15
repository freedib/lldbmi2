
#ifndef VARIABLES_H
#define VARIABLES_H

#include <lldb/API/LLDB.h>
using namespace lldb;

typedef enum
{
	NO_SUMMARY		= 0x1,
	FULL_SUMMARY	= 0x2,
} VariableDetails;


#define CHANGE_DEPTH_MAX    3		// maximum depth to check variables changed
#define WALK_DEPTH_MAX      9		// maximum depth to walk variables when searching expressions
#define ARRAY_MAX         200		// maximum number of children to display. must be a multiple of 8
#define CHILDREN_MAX      150		// limit of children to examine when walking in them


#define min(a,b) ((a) < (b) ? (a) : (b))

bool  getPeudoArrayVariable (SBFrame frame, const char *expression, SBValue &var);
bool  getStandardPathVariable (SBFrame frame, const char *expression, SBValue &var);
const char *getName ( SBValue &var);
char *strrstr (char *string, const char *find);
bool  getDirectPathVariable (SBFrame frame, const char *expression, SBValue *foundvar, SBValue &parent, int depth);
char *castexpression (SBFrame frame, const char *expression, char *newexpression, size_t expressionsize);

SBValue getVariable (SBFrame frame, const char *expression, bool tryDirect=true);
int     updateVarState (SBValue var, int depth);

char * formatExpressionPath (StringB &expressionpathdescB, SBValue var);
char * formatChildrenList (StringB &childrendescB, SBValue var, char *expression, int threadindexid, int &varnumchildren);
char * formatChangedList (StringB &changedescB, SBValue var, bool &separatorvisible, int depth);
char * formatVariables (StringB &varsdescB, SBValueList varslist);
char * formatSummary (StringB &summarydescB, SBValue var);
char * formatValue (StringB &varsdescB, SBValue var, VariableDetails details);

char * formatExpressionPath (SBValue var);
char * formatChildrenList (SBValue var, char *expression, int threadindexid, int &varnumchildren);
char * formatChangedList (SBValue var, bool &separatorvisible, int depth);
char * formatVariables (SBValueList varslist);
char * formatSummary (SBValue var);
char * formatValue (SBValue var, VariableDetails details);

#endif // VARIABLES_H
