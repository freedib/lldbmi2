
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


struct typeclass_name_pair {
	const char *name;
	TypeClass type;
};

struct typeclass_name_pair typeclass_names[] =
{
	{   "Invalid",             eTypeClassInvalid           },	//   0  (0u)
	{   "Array",               eTypeClassArray             },   //   1  (1u << 0)
	{   "BlockPointer",        eTypeClassBlockPointer      },   //   2  (1u << 1)
	{   "Builtin",             eTypeClassBuiltin           },   //   3  (1u << 2)
	{   "Class",               eTypeClassClass             },   //   4  (1u << 3)
	{   "ComplexFloat",        eTypeClassComplexFloat      },   //   5  (1u << 4)
	{   "ComplexInteger",      eTypeClassComplexInteger    },   //   6  (1u << 5)
	{   "Enumeration",         eTypeClassEnumeration       },   //   7  (1u << 6)
	{   "Function",            eTypeClassFunction          },   //   8  (1u << 7)
	{   "MemberPointer",       eTypeClassMemberPointer     },   //   9  (1u << 8)
	{   "ObjCObject",          eTypeClassObjCObject        },   //  10  (1u << 9)
	{   "ObjCInterface",       eTypeClassObjCInterface     },   //  11  (1u << 10)
	{   "ObjCObjectPointer",   eTypeClassObjCObjectPointer },   //  12  (1u << 11)
	{   "Pointer",             eTypeClassPointer           },   //  13  (1u << 12)
	{   "Reference",           eTypeClassReference         },   //  14  (1u << 13)
	{   "Struct",              eTypeClassStruct            },   //  15  (1u << 14)
	{   "Typedef",             eTypeClassTypedef           },   //  16  (1u << 15)
	{   "Union",               eTypeClassUnion             },   //  17  (1u << 16)
	{   "Vector",              eTypeClassVector            },   //  18  (1u << 17)
	// Define the last type class as the MSBit of a 32 bit value
	{   "Other",               eTypeClassOther             },   //  19  (1u << 31)
	// Define a mask that can be used for any type when finding types
	{   "Any",                 eTypeClassAny               },   //  20  (0xffffffffu)
};

static uint32_t num_typeclass = sizeof(typeclass_names) / sizeof (struct typeclass_name_pair);

const char *
getNameForTypeClass (TypeClass typeclass)
{
	if (typeclass==eTypeClassAny)
		return "Any";
	unsigned utc = typeclass;
	for (uint32_t itc=1; itc<num_typeclass-2; itc++) {
		if (utc&1)
			return typeclass_names[itc].name;
		utc >>= 1;
	}
	return typeclass_names[0].name;
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
