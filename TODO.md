# To do
- Improve installation and usage guides. **Done**
- Disabling breakpoint do not work. **Done**

# Bugs
- -break-insert messages attaching to a process (seems to work anyway)
- -gdb show language. should return c instead of auto (seems to work anyway)
- If hit the red square to stop a program that's already at a breakpoint, the debugger session hangs
- Some times when continuing debugger session locks up.  It sometimes complains about a broken debugger connection.
- Variables not updated after within a class after a single step through code that changes these class variables. Validated for global variables
