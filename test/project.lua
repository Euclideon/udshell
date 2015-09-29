project "eptest"
  kind "ConsoleApp"
  language "C++"
  flags { "StaticRuntime" }

  includedirs { "../3rdparty/googletest/" .. googletestPath .. "/include" }

  includedirs { "../public/include" }
  includedirs { "../kernel/src" }

  includedirs { "../ud/udPlatform/Include" }
  includedirs { "../ud/udPointCloud/Include" }

  files { "src/**.cpp", "src/**.h" }
  files { "project.lua" }

  links { "epkernel" }
  links { googletestPath }

  -- include common stuff
  dofile "../common-proj.lua"

  -- eptest goes to public/bin
  targetdir "../public/bin/%{cfg.buildcfg}_%{cfg.platform}"
