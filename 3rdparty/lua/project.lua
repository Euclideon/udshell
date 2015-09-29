
luaPath = "lua-5.3.1"

project(luaPath)
  kind "StaticLib"
  language "C"
  flags { "StaticRuntime", "OmitDefaultLibrary" }

  files { luaPath .. "/src/**" }
  removefiles { luaPath .. "/src/luac.c" }
  files { "project.lua" }

  includedirs { luaPath .. "/src" }

  -- include common stuff
  dofile "../../common-proj.lua"

  warnings "Off"
