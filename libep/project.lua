project "libep"
	kind "StaticLib"
	language "C++"
	flags { "StaticRuntime", "OmitDefaultLibrary" }

	files { "src/**" }
	files { "public/include/**" }
	files { "project.lua" }

	includedirs { "../ud/udPlatform/Include" }
	includedirs { "../kernel/src" }

	-- include common stuff
	dofile "../common-proj.lua"
