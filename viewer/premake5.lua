solution "epviewer"

	-- This hack just makes the VS project and also the makefile output their configurations in the idiomatic order
	if _ACTION == "gmake" then
		configurations { "Release", "Debug", "DebugOpt" }
	else
		configurations { "Debug", "DebugOpt", "Release" }
		platforms { "x64", "x86", "Clang" }
		configuration { "Clang" }
			toolset "msc-LLVM-vs2013"
			architecture "x86_64"
		configuration {}
	end

	startproject "epviewer"

	dofile "../ud/3rdParty/GL/glew/project.lua"
	-- common-proj.lua set objdir and targetdir, we'll reset them correctly for udviewer
	objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
	targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

	dofile "../ud/udPlatform/project.lua"
  -- common-proj.lua set objdir and targetdir, we'll reset them correctly for udviewer
	objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
	targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

	dofile "../ud/udPointCloud/project.lua"
  -- common-proj.lua set objdir and targetdir, we'll reset them correctly for udviewer
	objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
	targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

	dofile "../3rdparty/lua/project.lua"
  -- common-proj.lua set objdir and targetdir, we'll reset them correctly for udviewer
	objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
	targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"

	dofile "../kernel/project.lua"
  -- common-proj.lua set objdir and targetdir, we'll reset them correctly for udviewer
	objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
	targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"


	dofile "project.lua"
