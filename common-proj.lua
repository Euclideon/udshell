
-- include common UD stuff
dofile "ud/common-proj.lua"

flags { "C++11", "NoMinimalRebuild", "NoPCH" }
-- override with local settings
configuration { "Debug" }
	optimize "Off"

configuration {}

includedirs { "libep/public/include" }

-- we use exceptions
exceptionhandling "Default"

-- common-proj.lua set objdir and targetdir, we'll reset them correctly for udShell
objdir "int/%{cfg.buildcfg}_%{cfg.platform}"
targetdir "bin/%{cfg.buildcfg}_%{cfg.platform}"
