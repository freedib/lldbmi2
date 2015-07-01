# LLDB framework

You will need LLDB's header files to and LLDB library to build LLDBMI2. You will need  LLDB library to run LLDBMI2. You have 3 options:

1. Use apple framework version if Xcode is installed. LLDB Framework doesn't include headers files. You'll need to add them to the framework.

2. Build yourself LLDB as a framework (see scripts). Sometimes, this build doesn't
while the dynamic library build does.

3. Build yourself LLDB as dynamic library (see scripts).

# Build LLDBMI2

LLDBMI2 comes as an eclipse C++ project. It is not a real C++ program as it is structured as a C program. In build setting you will have to specify headers path for LLDB include.