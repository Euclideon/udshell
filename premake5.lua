require "qt"
qt = premake.extensions.qt

require "gitlab"

solution "epshell"

	-- This hack just makes the VS project and also the makefile output their configurations in the idiomatic order
	if _ACTION == "gmake" then
		configurations { "Release", "Debug", "DebugOpt", "ReleaseClang", "DebugClang", "DebugOptClang" }
		configuration { "*Clang" }
			toolset "clang"
		configuration {}
	else
		configurations { "Debug", "DebugOpt", "Release" }
		if _OS == "windows" then
			platforms { "x64", "Clang" }
			configuration { "Clang" }
				toolset "msc-LLVM-vs2013"
				architecture "x86_64"
			configuration {}
		end
	end

	startproject "epshell"

	defines { "EP_USE_QT" }

	group "libs"
		dofile "3rdparty/lua/project.lua"
		dofile "3rdparty/googletest/project.lua"

		dofile "ud/udPlatform/project.lua"
		dofile "ud/udPointCloud/project.lua"

	group ""
		dofile "libep/project.lua"
		dofile "kernel/project.lua"
		dofile "shell/project.lua"
		dofile "test/project.lua"

	group "plugins"
		dofile "plugin/viewer/project.lua"
