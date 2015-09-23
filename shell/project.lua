project "epshell"
	kind "WindowedApp"
	language "C++"
	flags { "StaticRuntime" }

	files { "src/**.cpp", "src/**.h" }
	files { "res/**.qrc", "res/**.qml" }
--		files { "**.pro", "**.pri" }
	files { "../premake5.lua", "project.lua" }

	includedirs { "../public/include" }
	includedirs { "../kernel/src" }
	includedirs { "../ud/udPlatform/Include" }
	includedirs { "../ud/udPointCloud/Include" }

	links { "udPlatform" }
	links { "udPointCloud" }
	links { "epkernel" }
	links { luaPath }

	qt.enable()
--		qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
	qtmodules { "core", "qml", "quick", "gui" }
	qtprefix "Qt5"
	pic "on"
	configuration { "windows", "Debug or DebugOpt" }
		qtsuffix "d"

	configuration { "windows" }
		links { "ws2_32.lib", "winmm.lib", "assimp.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib64" }
		disablewarnings { "4481", "4127" } -- silence some Qt warnings

	configuration { "linux" }
		links { "assimp" }

	configuration { }

	-- include common stuff
	dofile "../ud/common-proj.lua"

	exceptionhandling "Default"

	-- common-proj.lua set objdir and targetdir, we'll reset them correctly for udShell
	objdir "../int/%{cfg.buildcfg}_%{cfg.platform}"
	targetdir "../public/bin/%{cfg.buildcfg}_%{cfg.platform}"
