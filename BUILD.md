# Build LLDBMI2

You will need LLDB's header files and LLDB's library or framework to run it.

LLDB Framework provided with Xcode doesn't include headers files, and LLDB's headers extracted from LLVM's project tree (version 15) are provided for your convenience.

LLDB Framework provided with Xcode or Developer tools can be used to run LLDBMI2.

You can also build yourself LLDB and use the shared library generated.

LLDBMI2 comes as a CMake Eclipse C++ project, but can be built without Eclipse.

# Build and install with cmake (no eclipse needed)
The default installation directory is /usr/local/bin.
- In a shell window, import the GIT project: `git clone https://github.com/freedib/lldbmi2.git`.
  A sub-directory lldbmi2 is created
- Enter the directory and execute `bash build.sh`
- A build directory is created with the executable
- To install: `sudo bash build.sh install`

# Notes
- It is possible to specify a non standard framework or library location. See build.sh for options
- Mac framework (LLDB.framework) is searched on `/Applications/Xcode.app/Contents/SharedFrameworks` and
  `/Library/Developer/CommandLineTools/Library/PrivateFrameworks`
- Linux library (liblldb.so) is searched on `/usr/lib` and `/usr/local/lib`
- On Linux, you can install with apt. You need **lldb-XX** and **liblldb-XX**.
  You MUST specify the location of the debug-server with somthing like this:
  `export LLDB_DEBUGSERVER_PATH=$(which lldb-server)`

  or alternatively: You must create the following link: (package bug?)
  `sudo mkdir /usr/lib/bin; sudo ln -s /usr/bin/lldb-server-12 /usr/lib/bin/lldb-server-12.0.0`
 
# Build LLDBMI2 from Eclipse (if exending the code)
From an existing workspace, import the lldbmi2 project.
You can build it from Eclipse

# Test LLDBMI2 from Eclipse
Launch configurations are provided to test it. They requires the `tests` application which is provided and built with a build.sh option.
