project "viewer"
	kind "SharedLib"
	language "C++"
--	flags { }

	files { "src/**" }
	files { "qml/**" }
	files { "project.lua" }

	includedirs { "src" }

	links { "libep" }

	qt.enable()
--	qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
	qtmodules { "core", "qml", "quick", "gui" }
	qtprefix "Qt5"
	pic "on"
	configuration { "windows", "Debug* or DebugOpt*" }
		qtsuffix "d"

	configuration { "windows" }
		postbuildcommands {
			"IF NOT EXIST bin/plugins/ ( {MKDIR} bin/plugins/ )",
			"{COPY} %{cfg.buildtarget.abspath} bin/plugins/"
		}
	configuration { "not windows" }
		postbuildcommands {
			"{MKDIR} bin/plugins/",
			"{COPY} %{cfg.buildtarget.abspath} bin/plugins/"
		}

	configuration {}

	-- include common stuff
	dofile "../../common-proj.lua"


