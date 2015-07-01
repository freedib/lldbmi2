# LLDBMI2

A simple MI interface to LLDB.

# CONTEXT

Since Apple has withdrawn its support for GDB. The options to debug an application with CDT on Mac OS X are:

1. Use Xcode:
- Natural for Mac OS X, but limited for sharing projects or cross compiling on Linux or Windows.
- Not adequate for multi-language programs (eg: C with Java or Perl).

2. Install GNU GDB:
- Easy to install from Homebrew or Macports.
- Does not support Mac OS X dynamic libraries preventing from debugging code inside these libraries.

3. Install LLDB-MI:
- This program is promising, but is not yet mature.
- With Eclipse, it must be run in a manual remote debugging session implying to open a shell window and start manually a debug server with the program being debugged as argument (if there is a better way, doc do not mention it).
- The actual version doesn’t display nor update variables correctly.
- Many errror messages with Eclipse (command arguments nor recognized(.
- The code is complex and not easy to debug for a newcomer.

LLDBMI2 is a lightweight alternative to LLDB-MI on Mac OS X.
It should be useful until LLDB-MI gets enough maturity or better, if Eclipse supports directly LLDB.
It could be adapted to Linux or Windows, but not very useful while GDB is fully operational on these platforms.

# LIMITATIONS
No support for Non-stop debugging, Multi-process debugging and Reverse debugging until LLDB support them.