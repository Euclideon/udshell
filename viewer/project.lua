project "epviewer"
	kind "WindowedApp"
	language "C++"
	flags { "FatalCompileWarnings" }

	files { "src/**.cpp", "src/**.h" }
	files { "premake5.lua", "project.lua" }

	if _OS == "windows" then
		files { "viewer.rc", "resource.h" }
	end

	includedirs { "../public/include" }
	includedirs { "../kernel/src" }

	links { "GLEW" }
	links { "SDL2" }
	links { "udPlatform" }
	links { "udPointCloud" }
	links { "libep" }
	links { "hal-sdl" }
	links { "epkernel" }
	links { luaPath }
	links { "pcre" }

	defines { "GLEW_STATIC" }

	configuration { "windows" }
		links { "ws2_32.lib", "winmm.lib", "opengl32.lib", "glu32.lib" }
 		includedirs { "../ud/3rdParty/sdl2/include", "../up/3rdParty/GL/glew/include" }
	configuration { "windows", "x64" }
		links { "assimp-ep64.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib/windows/x64", "../ud/3rdparty/sdl2/lib/x64" }
		postbuildcommands {
			"{COPY} ud/3rdParty/sdl2/lib/x64/SDL2.dll .",
			"{COPY} 3rdparty/assimp-3.1.1/lib/windows/x64/assimp-ep64.dll ."
		}
	configuration { "windows", "x86" }
		links { "assimp-ep32.lib" }
		libdirs { "../3rdparty/assimp-3.1.1/lib/windows/x32", "../ud/3rdparty/sdl2/lib/x86" }
		postbuildcommands {
			"{COPY} ud/3rdParty/sdl2/lib/x86/SDL2.dll .",
			"{COPY} 3rdparty/assimp-3.1.1/lib/windows/x32/assimp-ep32.dll ."
		}

	configuration { "linux" }
		links { "assimp-ep", "GL", "dl" }
		libdirs { "../bin/amd64" }
		linkoptions { "-Wl,-rpath=/opt/Euclideon/Shell" }
		linkoptions { "-Wl,-rpath=../bin/amd64" }

	configuration { "windows", "x64", "Debug*" }
		linkoptions { "bin/dinkey/lib/dpwin64debug.obj" }
	configuration { "windows", "x64", "not Debug*" }
		linkoptions { "bin/dinkey/lib/dpwin64debug.obj" }
	configuration { "windows", "x86", "Debug*" }
		linkoptions { "bin/dinkey/lib/dpwin32_coff_debug.obj" }
	configuration { "windows", "x86", "not Debug*" }
		linkoptions { "bin/dinkey/lib/dpwin32_coff_debug.obj", "/SAFESEH:NO" }
	configuration { "linux", "Debug*" }
		links { "../bin/dinkey/linux/lib/dplin64debug.o" }
	configuration { "linux", "not Debug*" }
		links { "../bin/dinkey/linux/lib/dplin64debug.o" }

	configuration { }

	filter { "system:windows" }
		buildoptions { "/MP" }
	filter { }

	-- include common stuff
	dofile "../common-proj.lua"

	-- viewer goes to public/bin
	targetdir "../public/bin/%{cfg.buildcfg}_%{cfg.platform}"
