
#ifndef TEST_H
#define TEST_H

#include <lldb/API/LLDB.h>
using namespace lldb;

const char ** getTestCommands  (int test_sequence);
void          setTestSequence  (int ts);
void          setTestScript    (char *ts);
const char  * getTestCommand   ();
const char  * getTestSequenceCommand ();
const char  * getTestScriptCommand   ();

#endif // TEST_H
