project "udViewerLib"
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

	defines { "GLEW_STATIC" }

	configuration { "windows" }
		includedirs { "../3rdParty/sdl2/include" }

	-- include common stuff
	dofile "../common-proj.lua"
