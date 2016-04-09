# Debug with LLDBMI2

Once lldbmi2 is installed, 3 steps must be done to debug an application :

1. In `Debug Configurations...`, create a new `C/C++ Application` and change the `GDB debugger` (in `Debugger` tab) from **gdb** to **lldbmi2**. Options to lldbmi2 may be set there. Something like `/usr/local/bin/lldbmi2 --log`.

If you debugger don't work, you may have to specify library's path
2. In project's `Properties` (or best in `Eclipse preferences...`), `C/C++ Build` tab, `Environment` section, set the lldb's framework or dynamic library path which is required by lldbmi2. It can be done with environment variable DYLD_FRAMEWORK_PATH.
3. In project's properties (or best in Eclipse preferences), "C/C++ Build" tab, "Environment" section, set the path for **debugserver** which is required by lldb's library. It can be done with environment variable LLDB_DEBUGSERVER_PATH.

Note: If the environment is set in Eclipse preference, it will bet set for all projects once.
In my case it looks like:

    LLDB_DEBUGSERVER_PATH: /Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework/Resources/debugserver
    DYLD_FRAMEWORK_PATH:   /Applications/Xcode.app/Contents/SharedFrameworks

Three launch configuration are provided with a sample `tests` application.
- `tests lldbmi2` is the normal launch.
- `lldbmi2 test lldbmi2` is used to debug lldbmi2.
- `lldbmi2 script lldbmi2` is used to replay a log to debug lldbmi2.
