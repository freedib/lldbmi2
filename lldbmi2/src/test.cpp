
#include <stdio.h>
#include <unistd.h>

// test sequence to debug lldbmi2
// comment unwanted commands as you wish

// test commands will pause after a exec-run or exec-continue

const char *testcommands[] = {
//		"5-enable-pretty-printing",
//		"51-environment-cd /Users/didier/Projets/LLDB/hello/Debug",
//		"52-gdb-set solib-search-path /Users/didier/Projets/git-gasio/gasio/Debug",
		"52-gdb-set env DYLD_LIBRARY_PATH = /gasio/Debug",
//		"52-file-exec-and-symbols --thread-group i1 /Users/didier/Projets/LLDB/hello/Debug/hello",
//		"53-gdb-set --thread-group i1 args -h localhost",
//		"54-gdb-show --thread-group i1 language",
//		"55-interpreter-exec --thread-group i1 console \"p/x (char)-1\"",
//		"56-interpreter-exec --thread-group i1 console \"show endian\"",
//		"57-data-evaluate-expression \"sizeof (void*)\"",
		"58-break-insert --thread-group i1 /Users/didier/Projets/LLDB/hello/src/hello.c:69",
		"59-break-insert --thread-group i1 -t -f main",
//		"60-break-delete 2",
		"58-break-insert --thread-group i1 /Users/didier/Projets/LLDB/hello/src/hello.c:30",
		"61-inferior-tty-set --thread-group i1 %1",	// stdout instead of /dev/ptyxx
//		"62-exec-run --thread-group i1",
		"62-target-attach --thread-group i1 hello",
		"79-exec-continue --thread-group i1",
//		"37info sharedlibrary",
//		"63-list-thread-groups --available",
		"64-list-thread-groups",
		"65-list-thread-groups i1",
		"69-thread-info 1",
		"79-thread-info 2",
//		"71-var-create --thread 1 --frame 0 - * c",
//		"39-var-set-format c octal",
//		"76-var-evaluate-expression c+2",
		"77-exec-step --thread-group i1",
		"68-stack-info-depth --thread 1 11",
		"66-stack-list-frames --thread 1",
		"67-stack-list-locals --thread 1 --frame 0 1",
//		"72-var-create --thread 1 --frame 0 - * b",
//		"72-var-create --thread 1 --frame 0 - * z",
//		"73-var-list-children z",
//		"73-var-list-children (z)->y",
//		"73-var-list-children z.y",
//		"73-var-evaluate-expression z.y.a",
//		"73-var-evaluate-expression z.y->a",
//		"74-var-info-path-expression $0.*b",
//		"75-var-evaluate-expression $0.*b",
//		"38-data-evaluate-expression --thread 1 --frame 0 b",
//		"69-thread-info 1",
		"76-exec-next --thread-group i1",
		"78-exec-finish --thread-group i1",
		"79-exec-continue --thread-group i1",
		"67-stack-list-locals --thread 2 --frame 0 1",
		"68-stack-list-arguments --thread 2 --frame 0 1",
		"80-gdb-exit",
		NULL };
