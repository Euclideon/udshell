project "eptest"
	kind "ConsoleApp"
	language "C++"
	flags { "StaticRuntime" }

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
	links { "epkernel" }
	links { luaPath }
	links { googletestPath }

	configuration { "windows" }
		links { "ws2_32.lib", "winmm.lib", "assimp.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib64" }

if qt then
	qt.enable()
	--	qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
	qtmodules { "core", "qml", "quick", "gui" }
	qtprefix "Qt5"
	pic "on"
	configuration { "windows", "Debug or DebugOpt" }
	qtsuffix "d"
	disablewarnings { "4481", "4127" } -- silence some Qt warnings
 end

	-- include common stuff
	dofile "../common-proj.lua"

	-- eptest goes to public/bin
	targetdir "../public/bin/%{cfg.buildcfg}_%{cfg.platform}"
