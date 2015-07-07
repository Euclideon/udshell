project "udKernel"
	kind "StaticLib"
	language "C++"
	flags { "StaticRuntime", "OmitDefaultLibrary" }

	files { "Source/**", "Include/**" }
	files { "project.lua" }

	includedirs { "Include" }
	includedirs { "../udPlatform/Include" }
	includedirs { "../udPointCloud/Include" }
	includedirs { "../3rdParty/GL/freeglut/static/Include" }
	includedirs { "../3rdParty/GL/glew/include" }
	includedirs { "../3rdParty/lua/" .. luaPath .. "/src"}

	defines { "GLEW_STATIC" }

	configuration { "windows" }
		includedirs { "../3rdParty/sdl2/include" }

	configuration { "PNaCl" }
		buildoptions { "-std=gnu++11" }

	-- include common stuff
	dofile "../common-proj.lua"
