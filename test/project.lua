project "eptest"
	kind "ConsoleApp"
	language "C++"
--	flags { }

	includedirs { "../3rdparty/googletest/" .. googletestPath .. "/include" }

	includedirs { "../public/include" }
	includedirs { "../kernel/src" }
	includedirs { "../test/src" }

	includedirs { "../ud/udPlatform/Include" }
	includedirs { "../ud/udPointCloud/Include" }

	files { "src/**.cpp", "src/**.h" }
	files { "project.lua" }

	links { "udPlatform" }
	links { "udPointCloud" }
	links { "libep" }
	links { "hal-null" }
	links { "epkernel" }
	links { luaPath }
	links { "pcre" }
	links { googletestPath }

	links { "GLEW" }
	defines { "GLEW_STATIC" }

	configuration { "windows" }
		links { "ws2_32.lib", "winmm.lib", "opengl32.lib", "glu32.lib" }

	configuration { "windows", "x64" }
		links { "assimp-ep64.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib/windows/x64" }
	configuration { "windows", "x86" }
		links { "assimp-ep32.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib/windows/x32" }

	configuration { "linux" }
		libdirs { "../bin/amd64" }
		links { "assimp-ep", "GL", "dl" }
		linkoptions { "-Wl,-rpath=../bin/amd64,-rpath=bin/amd64" }

	configuration {}

if qt then
	qt.enable()
	--	qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
	qtmodules { "core", "qml", "quick", "gui" }
	qtprefix "Qt5"
	pic "on"
	configuration { "windows", "Debug*" }
	qtsuffix "d"
	disablewarnings { "4481", "4127" } -- silence some Qt warnings
 end

	filter { "system:windows" }
		buildoptions { "/MP" }
	filter { }

	-- include common stuff
	dofile "../common-proj.lua"

	-- eptest goes to public/bin
	targetdir "../public/bin/%{cfg.buildcfg}_%{cfg.platform}"
