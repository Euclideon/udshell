project "epviewer"
	kind "WindowedApp"
	language "C++"
	flags { "StaticRuntime" }

	files { "src/**.cpp", "src/**.h" }
	files { "../premake5.lua", "project.lua" }

	includedirs { "../ud/udPlatform/Include" }
	includedirs { "../ud/udPointCloud/Include" }
	includedirs { "../kernel/src" }

	links { "GLEW" }
	links { "SDL2" }
	links { "udPlatform" }
	links { "udPointCloud" }
	links { "epkernel" }
	links { luaPath }

	defines { "GLEW_STATIC" }


	configuration { "windows" }
		links { "ws2_32.lib", "winmm.lib", "opengl32.lib", "glu32.lib", "assimp.lib", "GLEW" }
 		includedirs { "../ud/3rdParty/sdl2/include", "../up/3rdParty/GL/glew/include" }
	configuration { "windows", "x64" }
		libdirs { "../3rdparty/assimp-3.1.1/lib64", "../ud/3rdparty/sdl2/lib/x64" }
		postbuildcommands { "postbuild.sh x64" }
	configuration { "windows", "x86" }
		libdirs { "../3rdparty/assimp-3.1.1/lib32", "../ud/3rdparty/sdl2/lib/x86" }
		postbuildcommands { "postbuild.sh x86" }

	configuration { "linux" }
		links { "assimp", "GL" }

	configuration { }

	-- include common stuff
	dofile "../ud/common-proj.lua"

	exceptionhandling "Default"

	-- common-proj.lua set objdir and targetdir, we'll reset them correctly for epviewer
	objdir "../int/%{cfg.buildcfg}_%{cfg.platform}"
	targetdir "../public/bin/%{cfg.buildcfg}_%{cfg.platform}"


