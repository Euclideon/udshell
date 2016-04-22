require "nacl"
require "emscripten"

solution "platform-web"

	platforms { "PNaCl" }

	-- This hack just makes the VS project and also the makefile output their configurations in the idiomatic order
	if _ACTION == "gmake" then
		configurations { "Release", "Debug", "DebugOpt" }
	else
		configurations { "Debug", "DebugOpt", "Release" }
	end

	startproject "viewer"

	group "libs"
		dofile "3rdparty/lua/project.lua"

		dofile "ud/udPlatform/project.lua"
		dofile "ud/udPointCloud/project.lua"

	group ""
		dofile "libep/project.lua"
		dofile "kernel/project.lua"
		dofile "viewer/project.lua"

--	group "plugins"
--		dofile "plugin/viewer/project.lua"
