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
	qtmodules { "core", "qml", "quick", "gui" }
	qtprefix "Qt5"
	pic "on"

	configuration { "windows", "Debug*" }
		qtsuffix "d"

	if _OS == "windows" then
	configuration { "windows", "x86" }
		local qtdir32 = os.getenv("QTDIR32") or os.getenv("QT_DIR32")
		qtpath(qtdir32)
		debugenvs { "PATH=" .. qtdir32 .. "\\bin;%PATH%" }
	end

	configuration { "windows" }
		postbuildcommands {
			"IF NOT EXIST bin/plugins/ {MKDIR} bin/plugins/",
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


