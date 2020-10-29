## Euclideon Platform (Discontinued) ##

> [Update 2020] This project as it stands references non-opensource projects and will not build out of the box. Some of the udPlatform files _may_ be available in the [udCore](https://github.com/Euclideon/udCore) repository. Various Dinkey protection files were removed from the history that contained secret keys.

### Overview ###

This repository contains the source to the Euclideon Platform series of projects.
These include:

 * *kernel* - Euclideon Platform Kernel (Static Lib)
 * *libep* - Public 'SDK' for developing EP apps and extentions (Static Lib + headers)
 * *hal* - Suite of hardware abstraction layer libraries to support various backends (Static Libs)
 * *shell* - Qt based Euclideon Platform Shell (Application)
 * *viewer* - SDL based viewer (Application)
 * *daemon* - Headless service host application; servers, RPC, script/cmd line (Application)

### Build Instructions ###

#### Setup ####

The EP set of projects use premake to generate platform/compiler specific build
files. Premake is included in the repository to simplify this process. We
currently support building on Windows and Linux with Visual Studio 2015, Clang
and GCC.

Before building, you will need to setup your build environment to include some
additional dependencies. The build runs a Bash script during compilation and
therefore requires [GitBash](https://git-for-windows.github.io/) to be installed
and in the system path in order to compile on Windows.

Additionally, The full EP build requires the
[Qt SDK](http://www.qt.io/download-open-source/) to be installed. We require at
least version 5.6 to compile shell. You will also need to set the `QTDIR`
environment variable to point to the root directory of the desired Qt toolchain
(eg. `C:\Qt5\5.6\msvc2015_64`). It's also recommended to add `%QTDIR%\bin` to
your system path to ensure the Qt dll's are found when running.

#### Build ####

Run the appropriate `create_project.bat` or `create_project.sh` script and
select your chosen build environment to generate the build files and associated
dependencies. Note that Clang is added as a configuration option for Visual
Studio project files. Visual Studio users should make sure to select 'shell',
'viewer', or 'daemon' as startup project to launch the correct application.

### Where we're at ###

At the time of discontinuation, EP is in a fairly capable prototype state.

 * Infrastructure and supporting libs (containers, math, etc) are mostly
   stabilised
 * UI integration is complete
 * Plugin system design and API are working, extensibility is working from
   several languages
 * Loading/saving project files and scenes is working
 * Integration with UD rendering library is fairly mature
 * Core GPU rendering features are available
 * Resource management is centralised, shared among active activity instances
   allowing associated activities to cooperate to form complex/custom workflows
 * Many poly-geom formats and texture formats are supported
 * Build infrastructure is capable of producing distributable builds for all
   supported platforms
 * Documentation is available in chm, html, pdf formats, but content is
   redimentary
 * Unit-test infrastructure is preliminary; tests some core utility libraries

### Planned future development ###

 * **Renderer improvements** - Rejig renderer to pull frames on demand rather than
have kernel push new frames for rendering when the renderer may-not be ready to
accept them. This will improve performance, and resolve various subtle problems
relating to scene dirty-state propagation.
 * **Compositor** - Implement a compositing API, allowing flexibility of complex
rendering setups involving layered UD/Geom sub-scenes and introducing
user-supplied post-effects. This is particularly useful for niche visualisation
tasks.
 * **Viewport gizmos** - A preliminary branch exists introducing a 'gizmo' API,
allowing users to produce components intended to interact with nodes in the
viewports/scenes. Includes initial implementations of translation/rotation/scaling
gizmos.
 * **GDAL integration** - Geolocation services are a requirement for geospatial
products.
 * **Undo/redo system** - This speaks for itself; a framework should exist for
managing a stack of destructive operations.
 * **Produce more out-of-the-box UI components** - Eg, a decent tree-view, general
property inspector, etc; UI components which may be shared among application UIs.
