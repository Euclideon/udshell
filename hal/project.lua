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
		defines { "EP_USE_QT" }
		configuration { "*QML" }
			defines { "QT_QML_DEBUG" }
		configuration {}

		qt.enable()
		qtmodules { "core", "core-private", "qml", "quick", "gui", "widgets" }
		qtprefix "Qt5"
		pic "on"

		configuration { "windows", "Debug*" }
			qtsuffix "d"

		if _OS == "windows" then
		configuration { "windows", "x86" }
			local qtdir32 = os.getenv("QTDIR32") or os.getenv("QT_DIR32")
			if qtdir32 ~= nil then
				qtpath(qtdir32)
				debugenvs { "PATH=" .. qtdir32 .. "\\bin;%PATH%" }
			end
		end

		configuration {}
	else
		if halBuild == "null" then
			defines { "EP_HEADLESS" }
		end
		defines { "GLEW_STATIC" }

--		includedirs { "../ud/3rdParty/GL/freeglut/static/Include" }
		includedirs { "../ud/3rdParty/sdl2/include" }
		includedirs { "../ud/3rdParty/GL/glew/include" }
	end
