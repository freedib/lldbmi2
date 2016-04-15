# Debug with LLDBMI2

Once lldbmi2 is installed, you can debug your application by creating a new `C/C++ Application` 
in `Debug Configurations...` and change the `GDB debugger` (in `Debugger` tab) from **gdb** to
 **lldbmi2**. Options to lldbmi2 may be set there. Something like `/usr/local/bin/lldbmi2 --log`.

Three launch configuration are provided with a sample `tests` application.
- `tests lldbmi2` is the normal launch.
- `lldbmi2 test lldbmi2` is used to debug lldbmi2.
- `lldbmi2 script lldbmi2` is used to replay a log to debug lldbmi2.

**Note**: Pointers variables are not always updated. To see their real values, click on then and check *Detail*. 