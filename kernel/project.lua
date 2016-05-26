project "epkernel"
	kind "StaticLib"
	language "C++"
	flags { "FatalCompileWarnings", "OmitDefaultLibrary" }

	files { "src/**" }
	files { "script/**" }
	files { "shaders/**" }
	files { "project.lua" }
	files { "text2c.sh" }

	includedirs { "src" }

	includedirs { "../hal/include" }

	includedirs { "../ud/udPlatform/Include" }
	includedirs { "../ud/udPointCloud/Include" }

	includedirs { "../3rdparty/lua/" .. luaPath .. "/src"}
	includedirs { "../3rdparty/assimp-3.1.1/include" }
	includedirs { "../3rdparty/rapidxml-1.13" }
	includedirs { "../3rdparty/rapidjson-1.0.2/include" }


	filter { "system:windows" }
		includedirs { "../3rdParty/pcre/include" }
		buildoptions { "/MP" }
	filter { }

	-- include common stuff
	dofile "../common-proj.lua"

	includedirs { "%{cfg.objdir}/script" }
	includedirs { "%{cfg.objdir}/shaders" }

	-- HACK: kernel seems to get the path wrong!
	includedirs { "%{cfg.objdir}/%{cfg.platform}/%{cfg.buildcfg}/%{prj.name}/script" }
	includedirs { "%{cfg.objdir}/%{cfg.platform}/%{cfg.buildcfg}/%{prj.name}/shaders" }

	filter "files:script/**"
		buildmessage 'text2c.sh %{file.relpath} %{cfg.objdir}/script/%{file.basename}.inc'
		buildcommands { path.getabsolute('.') .. '/text2c.sh "%{file.relpath}" "%{cfg.objdir}/script/%{file.basename}.inc" "%{file.name}"' }
		buildoutputs { '%{cfg.objdir}/script/%{file.basename}.inc' }
	filter "files:shaders/**"
		buildmessage 'text2c.sh %{file.relpath} %{cfg.objdir}/shaders/%{file.basename}.inc'
		buildcommands { path.getabsolute('.') .. '/text2c.sh "%{file.relpath}" "%{cfg.objdir}/shaders/%{file.basename}.inc" "%{file.name}"' }
		buildoutputs { '%{cfg.objdir}/shaders/%{file.basename}.inc' }
	filter {}
