
#include <stdio.h>
#include <unistd.h>

// test sequence to debug lldbmi2
// comment unwanted commands as you wish

// test commands will pause after a exec-run, exec-continue or target-attach

// remove comment above wanted commands set
//#define TEST_C
//#define TEST_C_ATTACH
//#define TEST_C_VARS
//#define TEST_C_SIMPLE
//#define TEST_C_POINTERS
//#define TEST_C_LARGE_ARRAY
//#define TEST_CPP_MEMBERS
#define TEST_CPP_STRING
//#define TEST_CPP_ARGS
//#define TEST_OTHER


#ifdef TEST_C
const char *testcommands[] = {
	"51-environment-cd /Users/didier/Projets/git-lldbmi2/test_hello_c/Debug",
	"52-file-exec-and-symbols --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_c/Debug/test_hello_c",
	"53-gdb-set --thread-group i1 args -h localhost",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_c/Sources/hello.c:77",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_c/Sources/hello.c:34",
	"61-inferior-tty-set --thread-group i1 %1",	// stdout instead of /dev/ptyxx
	"62-exec-run --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"79-thread-info 2",
	"77-exec-step --thread-group i1",
	"68-stack-info-depth --thread 1 11",
	"66-stack-list-frames --thread 1",
	"67-stack-list-locals --thread 1 --frame 0 1",
	"76-exec-next --thread-group i1",
	"78-exec-finish --thread-group i1",
	"79-exec-continue --thread-group i1",
	"67-stack-list-locals --thread 2 --frame 0 1",
	"68-stack-list-arguments --thread 2 --frame 0 1",
	"80-gdb-exit",
	NULL
};
#endif	// TEST_C

#ifdef TEST_C_VARS
const char *testcommands[] = {
	"51-environment-cd /Users/didier/Projets/git-lldbmi2/test_hello_c/Debug",
	"52-file-exec-and-symbols --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_c/Debug/test_hello_c",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_c/Sources/hello.c:74",
	"61-inferior-tty-set --thread-group i1 %1",	// stdout instead of /dev/ptyxx
	"62-exec-run --thread-group i1",
	"76-var-evaluate-expression c",
	"76-var-evaluate-expression pz->a",
	"76-var-evaluate-expression (pz)->a",
	"76-var-evaluate-expression pz.a",
	"80-gdb-exit",
	NULL
};
#endif	// TEST_C_VARS

#ifdef TEST_C_LARGE_ARRAY
const char *testcommands[] = {
	"3-environment-cd /Users/didier/Projets/git-lldbmi2/test_hello_c",
	"17-file-exec-and-symbols --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_c/Debug/test_hello_c",
//	"19-gdb-show --thread-group i1 language",
	"26-break-insert -f /Users/didier/Projets/git-lldbmi2/test_hello_c/Sources/hello.c:98",
	"28-exec-run --thread-group i1",
	"35-stack-list-locals --thread 1 --frame 0 1",
	"36-var-create --thread 1 --frame 0 - * c",
	"37-var-create --thread 1 --frame 0 - * &(c)",
	"38-stack-list-frames --thread 1",
	"39-var-create --thread 1 --frame 0 - * *((c)+0)@100",
	"39-var-create --thread 1 --frame 0 - * *((i)+0)@100",
	"39-var-create --thread 1 --frame 0 - * *((s)+0)@100",
	"40-exec-next --thread 1 1",
	"41-stack-list-locals --thread 1 --frame 0 1",
	"42-stack-info-depth --thread 1 11",
	"45-var-update 1 c",
	"46-var-update 1 $0",
	"47-var-update 1 c[0]",
	"40-exec-next --thread 1 1",
	"41-stack-list-locals --thread 1 --frame 0 1",
	"42-stack-info-depth --thread 1 11",
	"45-var-update 1 c",
	"46-var-update 1 $0",
	"47-var-update 1 c[0]",
	"59-exec-continue --thread 1",
	"80-gdb-exit",
	NULL
};
#endif	// TEST_C_LARGE_ARRAY

#ifdef TEST_C_POINTERS
const char *testcommands[] = {
	"51-environment-cd /Users/didier/Projets/git-lldbmi2/test_hello_c/Debug",
	"52-file-exec-and-symbols --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_c/Debug/test_hello_c",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_c/Sources/hello.c:118",
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
#endif	// TEST_C_POINTERS

#ifdef TEST_C_ATTACH
const char *testcommands[] = {
	"51-environment-cd /Users/didier/Projets/git-lldbmi2/test_hello_c/Debug",
	"62-target-attach --thread-group i1 test_hello_c",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_c/Sources/hello.c:34",
	"61-inferior-tty-set --thread-group i1 %1",	// stdout instead of /dev/ptyxx
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"79-thread-info 2",
	"22-gdb-show --thread-group i1 language",
	"67-stack-list-locals --thread 2 --frame 0 1",
	"68-stack-list-arguments --thread 2 --frame 0 1",
	"80-gdb-exit",
	NULL
};
#endif	// TEST_C_ATTACH

#ifdef TEST_CPP_MEMBERS
const char *testcommands[] = {
	"51-environment-cd /Users/didier/Projets/git-lldbmi2/test_hello_cpp/Debug",
	"52-file-exec-and-symbols --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_cpp/Debug/test_hello_cpp",
	"64-list-thread-groups",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_cpp/Sources/hello.cpp:49",
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
#endif	// TEST_CPP_MEMBERS

#ifdef TEST_CPP_STRING
const char *testcommands[] = {
	"51-environment-cd /Users/didier/Projets/git-lldbmi2/test_hello_cpp/Debug",
	"52-file-exec-and-symbols --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_cpp/Debug/test_hello_cpp",
	"64-list-thread-groups",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_cpp/Sources/hello.cpp:117",
	"62-exec-run --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",
	"72-var-create --thread 1 --frame 0 - * s",
	"73-var-list-children s",
	"73-var-list-children s.__r_",
	"107-var-list-children s.__r_.__first_",
	"104-var-list-children s.__r_.__first_.__s.(anonymous)",
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
#endif	// TEST_CPP_STRING

#ifdef TEST_CPP_ARGS
const char *testcommands[] = {
	"51-environment-cd /Users/didier/Projets/git-lldbmi2/test_hello_cpp/LLVM",
	"52-file-exec-and-symbols --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_cpp/LLVM/test_hello_cpp",
	"64-list-thread-groups",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_cpp/Sources/hello.cpp:74",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_cpp/Sources/hello.cpp:85",
	"58-break-insert --thread-group i1 /Users/didier/Projets/git-lldbmi2/test_hello_cpp/Sources/hello.cpp:87",
	"62-exec-run --thread-group i1",
	"64-list-thread-groups",
	"65-list-thread-groups i1",
	"69-thread-info 1",
	"67-stack-list-locals --thread 1 --frame 0 1",
	"38-var-create --thread 1 --frame 0 - * s",
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
	"38-var-create --thread 1 --frame 0 - * cdp",
	"38-var-create --thread 1 --frame 0 - * cdr",
	"67-var-list-children b",
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
#endif	// TEST_CPP_ARGS

#ifdef TEST_OTHER
const char *testcommands[] = {
	"5-enable-pretty-printing",
	"52-gdb-set solib-search-path /Users/didier/Projets/git-gasio/gasio/Debug",
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
#endif	// TEST_OTHER
