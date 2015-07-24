
#ifndef LANGUAGERUNTIME_H
#define LANGUAGERUNTIME_H

#include <lldb/API/LLDB.h>
using namespace lldb;

const char *getNameForLanguageType (LanguageType language);
const char *getNameForTypeClass (TypeClass typeclass);
const char *getNameForBasicType (BasicType basictype);

#endif // LANGUAGERUNTIME_H
