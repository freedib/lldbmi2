
#ifndef NAMES_H
#define NAMES_H

#include <lldb/API/LLDB.h>
using namespace lldb;

const char *getNameForLanguageType (LanguageType language);
const char *getNameForTypeClass (TypeClass typeclass);
const char *getNameForBasicType (BasicType basictype);

#endif // NAMES_H
