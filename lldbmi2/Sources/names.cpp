
// patch for SBLanguageRuntime::GetNameForLanguageType which was not included in earlier versions

#include "names.h"

struct languagetype_name_pair {
    const char *name;
    LanguageType type;
};

struct languagetype_name_pair languagetypes_names[] =
{
    // To allow GetNameForLanguageType to be a simple array lookup, the first
    // part of this array must follow enum LanguageType exactly.
    {   "unknown",          eLanguageTypeUnknown         },
    {   "c89",              eLanguageTypeC89             },
    {   "c",                eLanguageTypeC               },
    {   "ada83",            eLanguageTypeAda83           },
    {   "c++",              eLanguageTypeC_plus_plus     },
    {   "cobol74",          eLanguageTypeCobol74         },
    {   "cobol85",          eLanguageTypeCobol85         },
    {   "fortran77",        eLanguageTypeFortran77       },
    {   "fortran90",        eLanguageTypeFortran90       },
    {   "pascal83",         eLanguageTypePascal83        },
    {   "modula2",          eLanguageTypeModula2         },
    {   "java",             eLanguageTypeJava            },
    {   "c99",              eLanguageTypeC99             },
    {   "ada95",            eLanguageTypeAda95           },
    {   "fortran95",        eLanguageTypeFortran95       },
    {   "pli",              eLanguageTypePLI             },
    {   "objective-c",      eLanguageTypeObjC            },
    {   "objective-c++",    eLanguageTypeObjC_plus_plus  },
    {   "upc",              eLanguageTypeUPC             },
    {   "d",                eLanguageTypeD               },
    {   "python",           eLanguageTypePython          },
    {   "opencl",           eLanguageTypeOpenCL          },
    {   "go",               eLanguageTypeGo              },
    {   "modula3",          eLanguageTypeModula3         },
    {   "haskell",          eLanguageTypeHaskell         },
    {   "c++03",            eLanguageTypeC_plus_plus_03  },
    {   "c++11",            eLanguageTypeC_plus_plus_11  },
    {   "ocaml",            eLanguageTypeOCaml           },
    {   "rust",             eLanguageTypeRust            },
    {   "c11",              eLanguageTypeC11             },
    {   "swift",            eLanguageTypeSwift           },
    {   "julia",            eLanguageTypeJulia           },
    {   "dylan",            eLanguageTypeDylan           },
    {   "c++14",            eLanguageTypeC_plus_plus_14  },
    {   "fortran03",        eLanguageTypeFortran03       },
    {   "fortran08",        eLanguageTypeFortran08       },
    // Vendor Extensions
    {   "mipsassem",        eLanguageTypeMipsAssembler   },
    {   "renderscript",     eLanguageTypeExtRenderScript },
    // Now synonyms, in arbitrary order
    {   "objc",             eLanguageTypeObjC            },
    {   "objc++",           eLanguageTypeObjC_plus_plus  }
};

static uint32_t num_languagestypes = sizeof(languagetypes_names) / sizeof (struct languagetype_name_pair);

const char *
getNameForLanguageType (LanguageType languagetype)
{
    if (languagetype < num_languagestypes)
        return languagetypes_names[languagetype].name;
    else
        return languagetypes_names[eLanguageTypeUnknown].name;
}


struct basictype_name_pair {
    const char *name;
    BasicType type;
};

struct basictype_name_pair basictypes_names[] =
{
	    {   "Invalid",             eBasicTypeInvalid           },
	    {   "Void",                eBasicTypeVoid              },
	    {   "Char",                eBasicTypeChar              },
	    {   "SignedChar",          eBasicTypeSignedChar        },
	    {   "UnsignedChar",        eBasicTypeUnsignedChar      },
	    {   "WChar ",              eBasicTypeWChar             },
	    {   "SignedWChar ",        eBasicTypeSignedWChar       },
	    {   "UnsignedWChar",       eBasicTypeUnsignedWChar     },
	    {   "Char16",              eBasicTypeChar16            },
	    {   "Char32",              eBasicTypeChar32            },
	    {   "Short ",              eBasicTypeShort             },
	    {   "UnsignedShort",       eBasicTypeUnsignedShort     },
	    {   "Int ",                eBasicTypeInt               },
	    {   "UnsignedInt ",        eBasicTypeUnsignedInt       },
	    {   "Long",                eBasicTypeLong              },
	    {   "UnsignedLong",        eBasicTypeUnsignedLong      },
	    {   "LongLong  ",          eBasicTypeLongLong          },
	    {   "UnsignedLongLong",    eBasicTypeUnsignedLongLong  },
	    {   "Int128",              eBasicTypeInt128            },
	    {   "UnsignedInt128",      eBasicTypeUnsignedInt128    },
	    {   "Bool",                eBasicTypeBool              },
	    {   "Half",                eBasicTypeHalf              },
	    {   "Float ",              eBasicTypeFloat             },
	    {   "Double",              eBasicTypeDouble            },
	    {   "LongDouble",          eBasicTypeLongDouble        },
	    {   "FloatComplex",        eBasicTypeFloatComplex      },
	    {   "DoubleComplex",       eBasicTypeDoubleComplex     },
	    {   "LongDoubleComplex",   eBasicTypeLongDoubleComplex },
	    {   "ObjCID",              eBasicTypeObjCID            },
	    {   "ObjCClass",           eBasicTypeObjCClass         },
	    {   "ObjCSel",             eBasicTypeObjCSel           },
	    {   "NullPtr",             eBasicTypeNullPtr           },
	    {   "Other ",              eBasicTypeOther             },
};

static uint32_t num_basictypes = sizeof(basictypes_names) / sizeof (struct basictype_name_pair);

const char *
getNameForBasicType (BasicType basictype)
{
    if (basictype < num_basictypes)
        return basictypes_names[basictype].name;
    else
        return basictypes_names[eBasicTypeInvalid].name;
}

