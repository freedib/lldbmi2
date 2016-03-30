
#ifndef TEST_H
#define TEST_H

#include <lldb/API/LLDB.h>
using namespace lldb;

const char *  getTestDirectory ();
const char ** getTestCommands  (int test_sequence);

#endif // TEST_H
