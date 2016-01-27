project "viewer"
	kind "SharedLib"
	language "C++"
	flags { "StaticRuntime" }

	files { "src/**" }
	files { "res/**" }
	files { "project.lua" }

	includedirs { "src" }

	links { "libep" }

	qt.enable()
--	qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
	qtmodules { "core", "qml", "quick", "gui" }
	qtprefix "Qt5"
	pic "on"
	configuration { "windows", "Debug or DebugOpt" }
		qtsuffix "d"

	-- include common stuff
	dofile "../../common-proj.lua"


