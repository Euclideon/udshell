project "epshell"
	kind "WindowedApp"
	language "C++"

	flags { "FatalCompileWarnings" }

	files { "src/**.cpp", "src/**.h" }
	files { "qml/**.qrc", "qml/**.qml" }
	files { "res/**.qrc", "res/**.xml" }
	files { "../premake5.lua", "project.lua" }

	if _OS == "windows" then
		files { "shell.rc", "resource.h" }
	end

	includedirs { "../kernel/src" }

	links { "udPlatform" }
	links { "udPointCloud" }
	links { "libep" }
	links { "hal-qt" }
	links { "epkernel" }
	links { luaPath }
	links { "pcre" }
	dependson { "viewer" }

	defines { "EP_USE_QT" }
	configuration { "Debug* or ReleaseDev*" }
		defines { "QT_QML_DEBUG" }
	configuration {}

	qt.enable()
	qtmodules { "core", "qml", "quick", "gui", "widgets" }
	qtprefix "Qt5"
	pic "on"

	configuration { "windows", "Debug*" }
		qtsuffix "d"

	configuration { "windows" }
		links { "ws2_32.lib", "winmm.lib", "freetype265.lib" }
		disablewarnings { "4481", "4127" } -- silence some Qt warnings

	configuration { "windows", "x64" }
		links { "assimp-ep64.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib/windows/x64", "../3rdparty/freetype/lib/windows/x64" }
		local qtdir = os.getenv("QTDIR") or os.getenv("QT_DIR")
		if qtdir ~= nil then
			debugenvs { "PATH=" .. qtdir .. "\\bin;%PATH%" }
		end
	configuration { "windows", "x86" }
		if _OS == "windows" then
			local qtdir32 = os.getenv("QTDIR32") or os.getenv("QT_DIR32")
			if qtdir32 ~= nil then
				qtpath(qtdir32)
				debugenvs { "PATH=" .. qtdir32 .. "\\bin;%PATH%" }
			else
				prebuildcommands {
					"echo \"WIN32 build requires 32bit Qt toolchain. Please set QTDIR32 env variable\"",
					"exit -1"
				}
			end
		end
		links { "assimp-ep32.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib/windows/x32", "../3rdparty/freetype/lib/windows/x32" }

	configuration { "linux" }
		libdirs { "../bin/amd64" }
		links { "assimp-ep", "dl", "freetype" }
		linkoptions { "-Wl,-rpath=/opt/Euclideon/Shell" }
		linkoptions { "-Wl,-rpath=bin/amd64" }

	configuration { "windows", "x64", "ReleaseDev*" }
		linkoptions { "bin/dinkey/lib/dpwin64debug.obj" }
	configuration { "windows", "x86", "ReleaseDev*" }
		linkoptions { "bin/dinkey/lib/dpwin32_coff_debug.obj", "/SAFESEH:NO" }
	configuration { "linux", "ReleaseDev*" }
		links { "../bin/dinkey/linux/lib/dplin64debug.o" }

	configuration { }

	filter { "system:windows" }
		buildoptions { "/MP" }
	filter { }

	-- include common stuff
	dofile "../common-proj.lua"

	-- shell goes to public/bin
	targetdir "../public/bin/%{cfg.buildcfg}_%{cfg.platform}"
