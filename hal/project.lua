project("hal-" .. halBuild)
	kind "StaticLib"
	language "C++"
	flags { "FatalCompileWarnings", "OmitDefaultLibrary" }

	files { "include/**" }
	files { "src/**" }
	files { "project.lua" }

	includedirs { "src", "include" }
	includedirs { "../kernel/src" }

	-- TODO: can we remove these references?
	includedirs { "../ud/udPlatform/Include" }
	includedirs { "../ud/udPointCloud/Include" }

	filter "PNaCl"
		buildoptions { "-std=c++11" }
	filter {}

	filter { "system:windows" }
		buildoptions { "/MP" }
	filter {}

	-- include common stuff
	dofile "../common-proj.lua"

	if halBuild == "qt" then
		removeplatforms "x86"

		defines { "EP_USE_QT" }

		qt.enable()
--		qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
		qtmodules { "core", "core-private", "qml", "quick", "gui" }
		qtprefix "Qt5"
		pic "on"
		configuration { "windows", "Debug* or DebugOpt*" }
			qtsuffix "d"
		configuration {}
	else
		defines { "GLEW_STATIC" }

--		includedirs { "../ud/3rdParty/GL/freeglut/static/Include" }
		includedirs { "../ud/3rdParty/sdl2/include" }
		includedirs { "../ud/3rdParty/GL/glew/include" }

--		filter "windows"
--			includedirs { "../ud/3rdParty/sdl2/include" }
--		filter {}
	end
