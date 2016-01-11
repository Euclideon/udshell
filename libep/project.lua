project "libep"
	kind "StaticLib"
	language "C++"
	flags { "FatalCompileWarnings", "StaticRuntime", "OmitDefaultLibrary" }
	pic "on"

	files { "src/**" }
	files { "public/**" }
	files { "project.lua" }

	includedirs { "../ud/udPlatform/Include" }

	filter { "system:windows" }
		buildoptions { "/MP" }
	filter { }

	-- include common stuff
	dofile "../common-proj.lua"
