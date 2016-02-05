solution "epviewer"

	-- This hack just makes the VS project and also the makefile output their configurations in the idiomatic order
	if _ACTION == "gmake" then
		configurations { "Release", "Debug", "DebugOpt", "ReleaseClang", "DebugClang", "DebugOptClang" }
		configuration { "*Clang" }
			toolset "clang"
		configuration {}
	else
		configurations { "Debug", "DebugOpt", "Release" }
		if _OS == "windows" then
			platforms { "x64", "x86", "Clang" }
			configuration { "Clang" }
				toolset "msc-LLVM-vs2013"
				architecture "x86_64"
			configuration {}
		end
	end

	startproject "epviewer"

	group "libs"
		dofile "../ud/3rdParty/GL/glew/project.lua"
		-- common-proj.lua set objdir and targetdir, we'll reset them correctly for epviewer
		objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

		dofile "../ud/udPlatform/project.lua"
		objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

		dofile "../ud/udPointCloud/project.lua"
		objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

		dofile "../3rdparty/lua/project.lua"
		objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

		dofile "../3rdparty/googletest/project.lua"
		objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

	group ""
		dofile "../libep/project.lua"
		objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

		dofile "../kernel/project.lua"
		objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

		dofile "../test/project.lua"
		objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

		dofile "project.lua"

	group "plugins"
		-- dofile "../plugin/viewer/project.lua"
		objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
		targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

