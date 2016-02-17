project "epviewer"
	kind "WindowedApp"
	language "C++"
	flags { "FatalCompileWarnings", "StaticRuntime" }

	files { "src/**.cpp", "src/**.h" }
	files { "premake5.lua", "project.lua" }

	includedirs { "../public/include" }
	includedirs { "../kernel/src" }

	links { "GLEW" }
	links { "SDL2" }
	links { "udPlatform" }
	links { "udPointCloud" }
	links { "libep" }
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

	filter { "system:windows" }
		buildoptions { "/MP" }
	filter { }

	-- include common stuff
	dofile "../common-proj.lua"

	-- viewer goes to public/bin
	targetdir "../public/bin/%{cfg.buildcfg}_%{cfg.platform}"
