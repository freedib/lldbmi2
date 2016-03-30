# Build LLDBMI2

You will need LLDB's header files to LLDBMI2 and LLDB's framework to run it.

LLDB Framework provided with Xcode doesn't include headers files, and LLDB's headers extracted from LLVM's project tree are provided for your convenience.

LLDB Framework provided with Xcode can be used to run LLDBMI2.

LLDBMI2 comes as an eclipse C++ project.

# Import LLDBMI2

LLDBMI2 can be imported ...
- With GIT in shell windows
  - Create and navigate to your root project window
  - In a shell window type: `svn co https://github.com/freedib/lldbmi2.git lldbmi2`
  - From Eclipse, create a new workspace or use an existing one
  - Import the project with menu `File -> Import`. Select `General -> Existing projects in Workspace`, then navigate up to the imported tree. You will need to import each project individually (at least lldbmi2 and lldb-headers).
- From Eclipse with Egit
  - From Eclipse, create a new workspace or use an existing one
  - Import the project with menu `File -> Import`. Select `Git -> Projects from Git`, then click `Next` and `Clone URI`.
  - Copy `https://github.com/freedib/lldbmi2.git` in URI field, click `Next`, choose Master and click `Next`, `Next`, then let Eclipse import projects it found.

# Test LLDBMI2

A launch configuration is provided to test it. It requires the Hello test application (which must be built).

Note DYLD_FRAMEWORK_PATH and LLDB_DEBUGSERVER_PATH environment variables in launch configuration.

# Build and install with cmake (no eclipse needed)
The default installation directory is /usr/local/bin.
- Create directory where you want to build LLDBMI2
- From that directory run:
	- cmake \<path_to_project_git\>
	- make
	- make install
