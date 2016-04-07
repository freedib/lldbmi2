# Build LLDBMI2

You will need LLDB's header files to LLDBMI2 and LLDB's framework to run it.

LLDB Framework provided with Xcode doesn't include headers files, and LLDB's headers extracted from LLVM's project tree are provided for your convenience.

LLDB Framework provided with Xcode can be used to run LLDBMI2.

LLDBMI2 comes as an eclipse C++ project.

# Build LLDBMI2 from Eclipse
##LLDBMI2 can be imported ...
- With GIT in shell windows
  - From Eclipse, create a new workspace or use an existing one (e.g. /Users/**_user_**/lldbmi2workspace)
  - In a shell window, navigate to your workspace (e.g. /Users/**_user_**/lldbmi2workspace)
  - From the shell window, import the GIT project: `git clone https://github.com/freedib/lldbmi2.git lldbmi2`
  - From Eclipse, import the project with menu `File -> Import`. Select `General -> Existing projects in Workspace`, then navigate up to the imported tree (just click Browse). You will need to import all projects (at least lldbmi2 and lldb-headers).
- From Eclipse with Egit
  - From Eclipse, create a new workspace or use an existing one  (eg /Users/**_user_**/lldbmi2workspace)
  - Import the project with menu `File -> Import`. Select `Git -> Projects from Git`, then click `Next` and `Clone URI`.
  - Copy `https://github.com/freedib/lldbmi2.git` in URI field, click `Next`, choose Master and click `Next`.
  - Specify the destination directory (e.g. /Users/**_user_**/lldbmi2workspace/lldbmi2 then `Next` and `Next`, then let Eclipse import projects it found.

# Test LLDBMI2 from Eclipse

A launch configuration is provided to test it. It requires the Hello test application (which must be built).

Note DYLD_FRAMEWORK_PATH and LLDB_DEBUGSERVER_PATH environment variables in launch configuration.

# Build and install with cmake (no eclipse needed)
The default installation directory is /usr/local/bin.
- In a shell window, create a new root directory where you want to build LLDBMI2 (e.g. `mkdir ~/lldbmi2workspace`)
- Navigate to your workspace (e.g. `cd ~/lldbmi2workspace`)
- Import the GIT project: `git clone https://github.com/freedib/lldbmi2.git lldbmi2`. A sub-directory lldmi2 is created
- Create a build directory: e.g. `mkdir build`
- Navigate to build directory: e.g. `cd build`
- Cmake the project: `cmake ../lldbmi2`
- Build the project: `make`
- Install lldbmi2: `make install` (may need sudo)
