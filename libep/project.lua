project "libep"
	kind "StaticLib"
	language "C++"
	flags { "FatalCompileWarnings", "StaticRuntime", "OmitDefaultLibrary" }
	pic "on"

	files { "src/**" }
	files { "public/**" }
	files { "project.lua" }

	filter { "system:windows" }
		buildoptions { "/MP" }
	filter { }

	-- include common stuff
	dofile "../common-proj.lua"
