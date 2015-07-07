
// patch for SBLanguageRuntime::GetNameForLanguageType which was not included in earlier versions

#include "language.h"

struct language_name_pair {
    const char *name;
    LanguageType type;
};

struct language_name_pair language_names[] =
{
    // To allow GetNameForLanguageType to be a simple array lookup, the first
    // part of this array must follow enum LanguageType exactly.
    {   "unknown",          eLanguageTypeUnknown        },
    {   "c89",              eLanguageTypeC89            },
    {   "c",                eLanguageTypeC              },
    {   "ada83",            eLanguageTypeAda83          },
    {   "c++",              eLanguageTypeC_plus_plus    },
    {   "cobol74",          eLanguageTypeCobol74        },
    {   "cobol85",          eLanguageTypeCobol85        },
    {   "fortran77",        eLanguageTypeFortran77      },
    {   "fortran90",        eLanguageTypeFortran90      },
    {   "pascal83",         eLanguageTypePascal83       },
    {   "modula2",          eLanguageTypeModula2        },
    {   "java",             eLanguageTypeJava           },
    {   "c99",              eLanguageTypeC99            },
    {   "ada95",            eLanguageTypeAda95          },
    {   "fortran95",        eLanguageTypeFortran95      },
    {   "pli",              eLanguageTypePLI            },
    {   "objective-c",      eLanguageTypeObjC           },
    {   "objective-c++",    eLanguageTypeObjC_plus_plus },
    {   "upc",              eLanguageTypeUPC            },
    {   "d",                eLanguageTypeD              },
    {   "python",           eLanguageTypePython         },
    {   "opencl",           eLanguageTypeOpenCL         },
    {   "go",               eLanguageTypeGo             },
    {   "modula3",          eLanguageTypeModula3        },
    {   "haskell",          eLanguageTypeHaskell        },
    {   "c++03",            eLanguageTypeC_plus_plus_03 },
    {   "c++11",            eLanguageTypeC_plus_plus_11 },
    {   "ocaml",            eLanguageTypeOCaml          },
    {   "rust",             eLanguageTypeRust           },
    {   "c11",              eLanguageTypeC11            },
    {   "swift",            eLanguageTypeSwift          },
    {   "julia",            eLanguageTypeJulia          },
    {   "dylan",            eLanguageTypeDylan          },
    {   "c++14",            eLanguageTypeC_plus_plus_14 },
    {   "fortran03",        eLanguageTypeFortran03      },
    {   "fortran08",        eLanguageTypeFortran08      },
    // Vendor Extensions
    {   "mipsassem",        eLanguageTypeMipsAssembler  },
    {   "renderscript",     eLanguageTypeExtRenderScript},
    // Now synonyms, in arbitrary order
    {   "objc",             eLanguageTypeObjC           },
    {   "objc++",           eLanguageTypeObjC_plus_plus }
};

static uint32_t num_languages = sizeof(language_names) / sizeof (struct language_name_pair);

const char *
GetNameForLanguageType (LanguageType language)
{
    if (language < num_languages)
        return language_names[language].name;
    else
        return language_names[eLanguageTypeUnknown].name;
}

