project "udKernel"
	kind "StaticLib"
	language "C++"
	flags { "StaticRuntime", "OmitDefaultLibrary" }

	files { "Source/**", "Include/**" }
	files { "project.lua" }

	includedirs { "Include" }
	includedirs { "../udPlatform/Include" }
	includedirs { "../udPointCloud/Include" }
	includedirs { "../3rdParty/lua/" .. luaPath .. "/src"}

	if not qt then
		includedirs { "../3rdParty/GL/freeglut/static/Include" }
		includedirs { "../3rdParty/GL/glew/include" }
		defines { "GLEW_STATIC" }
	end

	configuration { "windows" }
		includedirs { "../3rdParty/sdl2/include" }

	configuration { "PNaCl" }
		buildoptions { "-std=c++11" }

	-- include common stuff
	dofile "../common-proj.lua"

	if qt then
		qt.enable()
--		qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
		qtmodules { "core", "qml", "quick", "gui" }
		qtprefix "Qt5"
		configuration { "Debug or DebugOpt" }
			qtsuffix "d"
	end
