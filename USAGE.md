# Debug with LLDBMI2

Once lldbmi2 is installed, you can debug your application by creating a new `C/C++ Application` 
in `Debug Configurations...` and change the `GDB debugger` (in `Debugger` tab) from **gdb** to
 **lldbmi2**. Options to lldbmi2 may be set there. Something like `/usr/local/bin/lldbmi2` or `/usr/local/bin/lldbmi2 --log`.

For multiple architecture applications (example running intel applications on arm) you may add the architecture name. e.g. `--arch x86_64`

**Note**: While debugging, pointers variables are not always updated. To see their real values, click on then and check *Detail*. 

Six configuration are provided with a sample `tests` application.
- `tests lldbmi2` is the normal launch to debug application test. test number is given as parameter of tests
- `lldbmi2 test lldbmi2` is used to debug lldbmi2 using application tests and a test number provided with --test.
- `lldbmi2 script *.txt` are commands sequences used to to debug lldbmi2 with %s variable substitution.
- `lldbmi2 script *.log` are used to replay a log to debug lldbmi2.
