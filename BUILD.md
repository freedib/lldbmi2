# Build LLDBMI2

You will need LLDB's header files to LLDBMI2 and LLDB's framework to run it.

LLDB Framework provided with Xcode doesn't include headers files, and LLDB's headers extracted from LLVM's project tree are provided for your convenience.

LLDB Framework provided with Xcode can be used to run LLDBMI2.

LLDBMI2 comes as an eclipse C++ project.

# Test LLDBMI2

A launch configuration is provided to test it. It requires the Hello test application (which must be built).

Note DYLD_FRAMEWORK_PATH and LLDB_DEBUGSERVER_PATH environment variables in launch configuration.

# Run LLDBMI2

No install script is provided. The binary file can be copied manually to /usr/local/bin.