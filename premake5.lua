require "qt"
qt = premake.extensions.qt

require "gitlab"

solution "epshell"

	-- This hack just makes the VS project and also the makefile output their configurations in the idiomatic order
	if _ACTION == "gmake" then
		configurations { "Release", "ReleaseDev", "Debug", "ReleaseClang", "ReleaseDevClang", "DebugClang" }
		configuration { "*Clang" }
			toolset "clang"
		configuration {}
		linkgroups 'On'
	else
		configurations { "Debug", "ReleaseDev", "Release" }
		if _OS == "windows" then
			platforms { "x64", "x86", "Clang" }
			configuration { "Clang" }
				toolset "msc-v140_clang_3_7"
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
		configuration { "Debug*" }
			optimize "Full"
		configuration {}

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
