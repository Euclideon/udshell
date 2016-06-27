require "qt"
qt = premake.extensions.qt

require "gitlab"

solution "epshell"

	-- This hack just makes the VS project and also the makefile output their configurations in the idiomatic order
	if _ACTION == "gmake" then
		configurations { "Release", "Debug", "DebugOpt", "DebugQML", "ReleaseQML", "ReleaseClang", "DebugClang", "DebugOptClang" }
		configuration { "*Clang" }
			toolset "clang"
		configuration {}
	else
		configurations { "Debug", "DebugOpt", "Release", "DebugQML", "ReleaseQML" }
		if _OS == "windows" then
			platforms { "x64", "x86", "Clang" }
			configuration { "Clang" }
				toolset "msc-LLVM-vs2013"
				architecture "x86_64"
			configuration {}
		end
	end

	startproject "epshell"

	group "libs"
		if _OS == "windows" then
			dofile "ud/3rdParty/GL/glew/project.lua"
			dofile "3rdparty/pcre/project.lua"
		end

		dofile "3rdparty/lua/project.lua"
		dofile "3rdparty/googletest/project.lua"

		dofile "ud/udPlatform/project.lua"
		dofile "ud/udPointCloud/project.lua"

	group "hal"
		halBuild = "qt"
		dofile "hal/project.lua"
		halBuild = "sdl"
		dofile "hal/project.lua"
		halBuild = "null"
		dofile "hal/project.lua"

	group ""
		dofile "libep/project.lua"
		dofile "kernel/project.lua"
		dofile "shell/project.lua"
		dofile "viewer/project.lua"
--		dofile "daemon/project.lua"
		dofile "test/project.lua"

	group "plugins"
		dofile "plugin/viewer/project.lua"
