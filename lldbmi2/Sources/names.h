
#ifndef LANGUAGERUNTIME_H
#define LANGUAGERUNTIME_H

#include <lldb/API/LLDB.h>
using namespace lldb;

const char *getNameForLanguageType (LanguageType language);
const char *getNameForBasicType (BasicType basictype);

#endif
