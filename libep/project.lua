project "libep"
	kind "StaticLib"
	language "C++"
	flags { "StaticRuntime", "OmitDefaultLibrary" }

	files { "src/**" }
	files { "public/**" }
	files { "project.lua" }

	includedirs { "../ud/udPlatform/Include" }

	-- include common stuff
	dofile "../common-proj.lua"
