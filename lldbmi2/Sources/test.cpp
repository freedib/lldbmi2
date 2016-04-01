
#include <stdio.h>
#include <unistd.h>

// test sequence to debug lldbmi2
// comment unwanted commands as you wish

// test commands will pause after a exec-run, exec-continue or target-attach

#define WITH_TESTS
#define WITH_LO_TESTS

const char *testcommands_NONE[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"61-inferior-tty-set --thread-group i1 %s",		// stdout instead of /dev/ptyxx
	"62-exec-run --thread-group i1",
	"80-gdb-exit",
	NULL
};

#ifdef WITH_TESTS

const char *testcommands_THREAD[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:34",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:77",
	"61-inferior-tty-set --thread-group i1 %s",		// stdout instead of /dev/ptyxx
	"62-exec-run --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"79-thread-info 2",
	"77-exec-step --thread-group i1",
	"68-stack-info-depth --thread 1 11",
	"66-stack-list-frames --thread 1",
	"67-stack-list-locals --thread 1 --frame 0 1",
	"73-var-evaluate-expression &(z->y->s)",
	"76-exec-next --thread-group i1",
	"78-exec-finish --thread-group i1",
	"79-exec-continue --thread-group i1",
	"67-stack-list-locals --thread 2 --frame 0 1",
	"68-stack-list-arguments --thread 2 --frame 0 1",
	"80-gdb-exit",
	NULL
};

const char *testcommands_VARS[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:74",
	"61-inferior-tty-set --thread-group i1 %s",		// stdout instead of /dev/ptyxx
	"62-exec-run --thread-group i1",
	"76-var-evaluate-expression c",
	"37-var-create --thread 1 --frame 0 - * pz",
	"76-var-evaluate-expression pz->a",
	"76-var-evaluate-expression (pz)->a",
	"76-var-evaluate-expression pz.a",
	"80-gdb-exit",
	NULL
};

const char *testcommands_UPDATE[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:76",
	"61-inferior-tty-set --thread-group i1 %s",		// stdout instead of /dev/ptyxx
	"62-exec-run --thread-group i1",
	"41-stack-list-locals --thread 1 --frame 0 1",
	"42-stack-info-depth --thread 1 11",
	"37-var-create --thread 1 --frame 0 - * py",
	"38-var-list-children py",
	"55-var-create --thread 1 --frame 0 - * py->s[0]",
	"56-var-info-path-expression py->m",
	"57-var-evaluate-expression py->m",
	"40-exec-next --thread 1 1",
	"41-stack-list-locals --thread 1 --frame 0 1",
	"42-stack-info-depth --thread 1 11",
	"45-var-update 1 py",
	"64-var-create --thread 1 --frame 0 - * strlen(b)",
	"80-gdb-exit",
	NULL
};

const char *testcommands_LARGE_CHAR_ARRAY[] = {
	"51-environment-cd %s/tests",
	"17-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
//	"19-gdb-show --thread-group i1 language",
	"53-gdb-set --thread-group i1 args %s",
	"26-break-insert -f %s/tests/Sources/tests.cpp:92",
	"28-exec-run --thread-group i1",
	"35-stack-list-locals --thread 1 --frame 0 1",
	"36-var-create --thread 1 --frame 0 - * ccc",
	"37-var-create --thread 1 --frame 0 - * &(ccc)",
	"38-stack-list-frames --thread 1",
	"39-var-create --thread 1 --frame 0 - * *((ccc)+0)@100",
	"39-var-create --thread 1 --frame 0 - * *((ccc)+100)@2",
	"40-exec-next --thread 1 1",
	"41-stack-list-locals --thread 1 --frame 0 1",
	"42-stack-info-depth --thread 1 11",
	"45-var-update 1 ccc",
	"46-var-update 1 $0",
	"47-var-update 1 c[100]",
	"80-gdb-exit",
	NULL
};

const char *testcommands_LARGE_ARRAY[] = {
	"51-environment-cd %s/tests",
	"17-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
//	"19-gdb-show --thread-group i1 language",
	"53-gdb-set --thread-group i1 args %s",
	"26-break-insert -f %s/tests/Sources/tests.cpp:112",
	"28-exec-run --thread-group i1",
	"39-var-create --thread 1 --frame 0 - * *((i)+0)@100",
	"39-var-create --thread 1 --frame 0 - * *((s)+0)@100",
	"80-gdb-exit",
	NULL
};

const char *testcommands_POINTERS[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:125",
	"62-exec-run --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",	// ^done,locals=[{name="ab",value="{a = 0, b = 0}"}]
	"72-var-create --thread 1 --frame 0 - * x",
	"78-var-update 1 x",
	"79-var-update 1 px",
	"77-exec-next --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",	// ^done,locals=[{name="ab",value="{a = 0, b = 0}"}]
	"78-var-update 1 x",
	"79-var-update 1 px",
	"77-exec-next --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",	// ^done,locals=[{name="ab",value="{a = 0, b = 0}"}]
	"78-var-update 1 x",
	"79-var-update 1 px",
	"77-exec-next --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",	// ^done,locals=[{name="ab",value="{a = 0, b = 0}"}]
	"78-var-update 1 x",
	"79-var-update 1 px",
//	"76-var-evaluate-expression pz->a",
//	"76-var-evaluate-expression (pz)->a",
//	"76-var-evaluate-expression pz.a",
	"80-gdb-exit",
	NULL
};

const char *testcommands_ATTACH[] = {
	"51-environment-cd %s/tests",
	"62-target-attach --thread-group i1 tests",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:34",
	"61-inferior-tty-set --thread-group i1 %s",		// stdout instead of /dev/ptyxx
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"79-thread-info 2",
	"22-gdb-show --thread-group i1 language",
	"67-stack-list-locals --thread 2 --frame 0 1",
	"68-stack-list-arguments --thread 2 --frame 0 1",
	"80-gdb-exit",
	NULL
};

const char *testcommands_MEMBERS[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"64-list-thread-groups",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:175",
	"62-exec-run --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",	// ^done,locals=[{name="ab",value="{a = 0, b = 0}"}]
	"72-var-create --thread 1 --frame 0 - * ab",	// ^done,name="ab",numchild="1",value="{...}",type="AB",thread-id="1",has_more="0"
	"73-var-list-children ab",						// ^done,numchild="1",children=[child={name="ab.public",exp="public",numchild="1",thread-id="1"}],has_more="0"
	"73-var-list-children ab.public",				// ^done,numchild="2",children=[child={name="ab.public",exp="public",numchild="1",thread-id="1"}],has_more="0"
	"77-exec-next --thread-group i1",
	"78-var-update 1 test",
	"77-exec-next --thread-group i1",
	"77-exec-next --thread-group i1",
	"77-exec-next --thread-group i1",
	"80-gdb-exit",
	NULL
};

const char *testcommands_STRING[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"64-list-thread-groups",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:263",
	"62-exec-run --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",
	"72-var-create --thread 1 --frame 0 - * s",
	"73-var-list-children s",
	"73-var-list-children s.__r_",
	"73-var-list-children s.__r_.__first_",
	"73-var-list-children s.__r_.__first_.__s.(anonymous)",
	"73-var-list-children \"s.__r_.std::__1::__libcpp_compressed_pair_imp<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::__rep, std::__1::allocator<char>, 2>\"",
	"73-var-list-children \"s.__r_.std::__1::__libcpp_compressed_pair_imp<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::__rep, std::__1::allocator<char>, 2>.__first_\"",
	"72-var-create --thread 1 --frame 0 - * cc",
	"73-var-list-children \"cc.data.std::__1::__vector_base<unsigned char, std::__1::allocator<unsigned char> >\"",
	"50-data-evaluate-expression --thread 1 --frame 0 \"cc.data.std::__1::__vector_base<unsigned char, std::__1::allocator<unsigned char> >\"",
	"77-exec-next --thread-group i1",
	"78-var-update 1 cc",
	"77-exec-next --thread-group i1",
	"77-exec-next --thread-group i1",
	"77-exec-next --thread-group i1",
	"80-gdb-exit",
	NULL
};

const char *testcommands_ARGS[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"64-list-thread-groups",
	"19-gdb-show --thread-group i1 language",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:210",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:224",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:226",
	"62-exec-run --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",
	"38-var-create --thread 1 --frame 0 - * s",
	"38-var-create --thread 1 --frame 0 - * s[0]",
	"38-var-create --thread 1 --frame 0 - * ab",
	"57-var-list-children ab",
	"58-var-info-path-expression ab.BA",
	"63-data-evaluate-expression --thread 1 --frame 0 ab.BA",
	"63-var-evaluate-expression ab.BA",
	"57-var-list-children ab.BA",
	"77-exec-continue --thread 1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",	// ^done,locals=[{name="ab",value="{a = 0, b = 0}"}]
	"38-var-create --thread 1 --frame 0 - * i",
	"38-var-create --thread 1 --frame 0 - * s",
	"38-var-create --thread 1 --frame 0 - * v",
	"38-var-create --thread 1 --frame 0 - * d",
	"38-var-create --thread 1 --frame 0 - * b",
	"67-var-list-children b",
	"38-var-create --thread 1 --frame 0 - * cdp",
	"38-var-create --thread 1 --frame 0 - * cdr",
	"77-exec-continue --thread 1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",
	"45-var-update 1 s",
	"77-exec-continue --thread 1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",	// ^done,locals=[{name="ab",value="{a = 0, b = 0}"}]
	"45-var-update 1 i",
	"45-var-update 1 s",
	"45-var-update 1 v",
	"45-var-update 1 d",
	"45-var-update 1 b",
	"45-var-update 1 cdp",
	"45-var-update 1 cdr",
	"77-exec-continue --thread 1",
	"80-gdb-exit",
	NULL
};

const char *testcommands_OTHER[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"5-enable-pretty-printing",
	"52-gdb-set solib-search-path %s/../git-gasio/gasio/Debug",
	"52-gdb-set env DYLD_LIBRARY_PATH = /gasio/Debug",
	"54-gdb-show --thread-group i1 language",
	"55-interpreter-exec --thread-group i1 console \"p/x (char)-1\"",
	"56-interpreter-exec --thread-group i1 console \"show endian\"",
	"57-data-evaluate-expression \"sizeof (void*)\"",
	"59-break-insert --thread-group i1 -t -f main",
	"60-break-delete 2",
	"37info sharedlibrary",
	"63-list-thread-groups --available",
	"71-var-create --thread 1 --frame 0 - * c",
	"39-var-set-format c octal",
	"76-var-evaluate-expression c+2",
	"72-var-create --thread 1 --frame 0 - * b",
	"72-var-create --thread 1 --frame 0 - * z",
	"73-var-list-children z",
	"73-var-list-children (z)->y",
	"73-var-list-children z.y",
	"73-var-evaluate-expression z.y.a",
	"73-var-evaluate-expression z.y->a",
	"74-var-info-path-expression $0.*b",
	"75-var-evaluate-expression $0.*b",
	"38-data-evaluate-expression --thread 1 --frame 0 b",
	"69-thread-info 1",
	"80-gdb-exit",
	NULL
};

const char *testcommands_CRASH[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:202",
	"61-inferior-tty-set --thread-group i1 %s",		// stdout instead of /dev/ptyxx
	"62-exec-run --thread-group i1",
	"76-var-evaluate-expression err",
	"80-gdb-exit",
	NULL
};

const char *testcommands_INPUT[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"58-break-insert --thread-group i1 %s/tests/Sources/tests.cpp:274",
	"61-inferior-tty-set --thread-group i1 %s",		// stdout instead of /dev/ptyxx
	"62-exec-run --thread-group i1",
	"80-gdb-exit",
	NULL
};

const char *testcommands_CATCH_THROW[] = {
	"51-environment-cd %s/tests",
	"52-file-exec-and-symbols --thread-group i1 %s/tests/Debug/tests",
	"53-gdb-set --thread-group i1 args %s",
	"61-inferior-tty-set --thread-group i1 %s",		// stdout instead of /dev/ptyxx
	"41catch catch",
	"62-exec-run --thread-group i1",
	"53-stack-list-frames --thread 1",
	"80-gdb-exit",
	NULL
};

#endif // WITH_TESTS

#ifdef WITH_LO_TESTS

// paths specific to developer's computer
const char *testcommands_LO[] = {
	"51-environment-cd /pro/lo/libreoffice",
	"52-file-exec-and-symbols --thread-group i1 /pro/lo/libreoffice/instdir/LibreOfficeDev.app/Contents/MacOS/soffice",
	"53-gdb-set --thread-group i1 args /Users/didier/Projets/LO/documents/Archive.odg",
	"58-break-insert --thread-group i1 XMLShapeImportHelper::CreateGroupChildContext",	// shapeimport.css:540
	"58-break-insert --thread-group i1 /pro/lo/libreoffice/xmloff/source/draw/shapeimport.cxx:540",
//	"58-break-insert --thread-group i1 SdXMLGenericPageContext::CreateChildContext",	// ximppage.cxx:262
//	"58-break-insert --thread-group i1 SdXMLTableShapeContext::StartElement",			// ximpbody.cxx:253
	"61-inferior-tty-set --thread-group i1 %s",		// stdout instead of /dev/ptyxx
	"62-exec-run --thread-group i1",
	"53-stack-list-frames --thread 1",
	"259-stack-list-locals --thread 1 --frame 1 1",
	"261-var-create --thread 1 --frame 1 - * this",
	"261-var-create --thread 1 --frame 1 - * nPrefix",
	"261-var-create --thread 1 --frame 1 - * rLocalName",
	"261-var-create --thread 1 --frame 1 - * xAttrList",
	"261-var-create --thread 1 --frame 1 - * pContext",
	"266-var-list-children this",
	"270-var-info-path-expression this->maStyleName",
	"280-var-list-children this->maName",
	"283-var-list-children this->maName.pData",
	"286-var-info-path-expression this->maName.pData->buffer",
	"289-var-create --thread 1 --frame 1 - * &(this->maName.pData->buffer)",
	"290-data-evaluate-expression --thread 1 --frame 1 this->maName.pData->buffer",
	"338-var-create --thread 1 --frame 1 - * this->maName.pData->buffer[0]",
	"80-gdb-exit",
	NULL
};

#endif // WITH_LO_TESTS


const char **
getTestCommands (int test_sequence)
{
	switch (test_sequence) {
#ifdef WITH_TESTS
	case 1:		return testcommands_THREAD;
	case 2:		return testcommands_VARS;
	case 3:		return testcommands_UPDATE;
	case 4:		return testcommands_LARGE_CHAR_ARRAY;
	case 5:		return testcommands_LARGE_ARRAY;
	case 6:		return testcommands_POINTERS;
	case 7:		return testcommands_ATTACH;
	case 8:		return testcommands_MEMBERS;
	case 9:		return testcommands_STRING;
	case 10:	return testcommands_ARGS;
	case 11:	return testcommands_CRASH;
	case 12:	return testcommands_INPUT;
	case 13:	return testcommands_CATCH_THROW;
	case 14:	return testcommands_OTHER;
#endif // WITH_TESTS
#ifdef WITH_LO_TESTS
	case 15:	return testcommands_LO;
#endif // WITH_LO_TESTS
	default:	return testcommands_NONE;
	}
}
