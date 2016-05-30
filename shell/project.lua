project "epshell"
	kind "WindowedApp"
	language "C++"

	removeplatforms "x86"

	flags { "FatalCompileWarnings" }

	files { "src/**.cpp", "src/**.h" }
	files { "res/**.qrc", "res/**.qml" }
	files { "../premake5.lua", "project.lua" }

	includedirs { "../public/include" }
	includedirs { "../kernel/src" }

	links { "udPlatform" }
	links { "udPointCloud" }
	links { "libep" }
	links { "hal-qt" }
	links { "epkernel" }
	links { luaPath }
	links { "pcre" }

	defines { "EP_USE_QT" }
	configuration { "*QML" }
		defines { "QT_QML_DEBUG" }
	configuration {}

	qt.enable()
--	qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
	qtmodules { "core", "qml", "quick", "gui" }
	qtprefix "Qt5"
	pic "on"
	configuration { "windows", "Debug*" }
		qtsuffix "d"

	configuration { "windows" }
		links { "ws2_32.lib", "winmm.lib" }
		disablewarnings { "4481", "4127" } -- silence some Qt warnings

	configuration { "windows", "x64" }
		links { "assimp-ep64.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib/windows/x64" }
	configuration { "windows", "x86" }
		links { "assimp-ep32.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib/windows/x32" }

	configuration { "linux" }
		libdirs { "../bin/amd64" }
		links { "assimp-ep", "dl" }
		linkoptions { "-Wl,-rpath=bin/amd64" }

	configuration { }

	filter { "system:windows" }
		buildoptions { "/MP" }
	filter { }

	-- include common stuff
	dofile "../common-proj.lua"

	-- shell goes to public/bin
	targetdir "../public/bin/%{cfg.buildcfg}_%{cfg.platform}"
