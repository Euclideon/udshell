## Euclideon Platform ##

### Overview ###

This repository contains the source to the Euclideon Platform series of projects.
These include:

 * *epKernel* - Euclideon Platform Kernel (Static Lib)
 * *epShell* - Qt based Euclideon Platform Shell (Application)
 * *epViewer* - SDL based UD viewer (Application)

### Build Instructions ###

#### Setup ####

The EP set of projects use premake to generate platform/compiler specific build
files. Premake is included in the repository to simplify this process. We
currently support building on Windows and Linux with Visual Studio 2010-2013, 
Clang and GCC.

Before building, you will need to setup your build environment to include some
additional dependencies. epKernel runs a Bash script during compilation and
therefore requires [GitBash](https://git-for-windows.github.io/) to be installed
and in the system path in order to compile on Windows.

Additionally, epShell requires the 
[Qt SDK](http://www.qt.io/download-open-source/) to be installed. We require at 
least version 5.3 to compile epShell. You will also need to set the *QTDIR*
environment variable to point to the root directory of the desired Qt toolchain
(eg. **C:\Qt5\5.4\msvc2013_64_opengl**). It's also recommended to add
*%QTDIR%\bin* to your system path to ensure the Qt dll's are found when running.

#### Build ####

Run the system appropriate *create_project* script and select your chosen
build environment to generate the build files for epShell and associated
dependencies. Note that Clang is added as a configuration option for Visual 
Studio project files. To generate the build files for epViewer run the 
*create_project* script located under *viewer*.