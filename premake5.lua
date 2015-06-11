require "qt"
local qt = premake.extensions.qt

solution "udShell"

	-- This hack just makes the VS project and also the makefile output their configurations in the idiomatic order
	if _ACTION == "gmake" then
		configurations { "Release", "Debug", "DebugOpt" }
	else
		configurations { "Debug", "DebugOpt", "Release" }
		platforms { "x64" }
	end

	startproject "udShell"

	defines { "UD_USE_QT" }

	if os.get() == "windows" then
--		dofile "ud/3rdParty/GL/glew/project.lua"
	end
	dofile "ud/udPlatform/project.lua"
	dofile "ud/udPointCloud/project.lua"
	dofile "ud/udViewerLib/project.lua"

	project "udShell"
		kind "WindowedApp"
		language "C++"
		flags { "StaticRuntime" }

		objdir "Output/intermediate/%{prj.name}/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "Output/bin/%{cfg.buildcfg}_%{cfg.platform}"

		files { "src/**.cpp", "src/**.h" }
		files { "res/**.qrc", "res/**.qml" }
--		files { "**.pro", "**.pri" }
		files { "premake5.lua" }

		includedirs { "ud/udPlatform/Include" }
		includedirs { "ud/udPointCloud/Include" }
		includedirs { "ud/udViewerLib/Include" }
		includedirs { "ud/3rdParty" }
--		includedirs { "ud/3rdParty/GL/glew/include" }

--		links { "GLEW" }
		links { "udPlatform" }
		links { "udPointCloud" }
		links { "udViewerLib" }

--		defines { "GLEW_STATIC" }

		qt.enable()
--		qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
		qtmodules { "core", "qml", "quick", "gui" }
		qtprefix "Qt5"

		configuration { "windows" }
			links { "ws2_32.lib", "opengl32.lib", "glu32.lib", "winmm.lib" }

		-- include common stuff
		dofile "ud/common-proj.lua"

		configuration { }
