project "vieweractivity"
	kind "SharedLib"
	language "C++"
	flags { "StaticRuntime" }

	files { "src/**" }
	files { "project.lua" }

	includedirs { "src" }

	links { "libep" }

	-- include common stuff
	dofile "../../common-proj.lua"
