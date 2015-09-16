require "qt"
qt = premake.extensions.qt

solution "udShell"

	-- This hack just makes the VS project and also the makefile output their configurations in the idiomatic order
	if _ACTION == "gmake" then
		configurations { "Release", "Debug", "DebugOpt" }
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

	startproject "udShell"

	defines { "UD_USE_QT" }

	dofile "ud/udPlatform/project.lua"
	dofile "ud/udPointCloud/project.lua"

	dofile "3rdParty/lua/project.lua"

	dofile "udKernel/project.lua"

	dofile "udShell/project.lua"
