project "vieweractivity"
	kind "SharedLib"
	language "C++"
	flags { "StaticRuntime" }

	files { "src/**" }
	files { "project.lua" }

	includedirs { "src" }
	includedirs { "../../ud/udPlatform/Include" } -- HAX HAX!! REMOVE THIS!

	links { "libep" }

	-- include common stuff
	dofile "../../common-proj.lua"
