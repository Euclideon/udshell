
pcrePath = "pcre-8.38"

project "pcre"
  kind "StaticLib"
  language "C"
  flags { "OmitDefaultLibrary" }

  files { pcrePath .. "/*.c", pcrePath .. "/*.h" }
  files { "include/**" }
  files { "project.lua" }

  includedirs { "include", "src" }

  defines {
    'HAVE_CONFIG_H',
    '_CRT_SECURE_NO_DEPRECATE',
    '_CRT_SECURE_NO_WARNINGS'
  }

  -- include common stuff
  dofile "../../common-proj.lua"

--  warnings "Off"
