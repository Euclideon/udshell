project "udKernel"
	kind "StaticLib"
	language "C++"
	flags { "StaticRuntime", "OmitDefaultLibrary" }

	files { "src/**" }
	files { "script/**" }
	files { "shaders/**" }
	files { "project.lua" }
	files { "text2c.sh" }

	includedirs { "src" }
	includedirs { "%{cfg.objdir}/script" }
	includedirs { "%{cfg.objdir}/shaders" }
	includedirs { "../ud/udPlatform/Include" }
	includedirs { "../ud/udPointCloud/Include" }
	includedirs { "../3rdParty/lua/" .. luaPath .. "/src"}
	includedirs { "../3rdParty/assimp-3.1.1/include" }

	if not qt then
--		includedirs { "../ud/3rdParty/GL/freeglut/static/Include" }
		includedirs { "../ud/3rdParty/sdl2/include" }
		includedirs { "../ud/3rdParty/GL/glew/include" }
		defines { "GLEW_STATIC" }
	end

	filter "windows"
		includedirs { "../ud/3rdParty/sdl2/include" }
	filter "PNaCl"
		buildoptions { "-std=c++11" }
	filter "files:script/**"
		buildmessage 'text2c.sh %{file.relpath} %{cfg.objdir}/script/%{file.basename}.inc'
		buildcommands { path.getabsolute('.') .. '/text2c.sh "%{file.relpath}" "%{cfg.objdir}/script/%{file.basename}.inc"' }
		buildoutputs { '%{cfg.objdir}/script/%{file.basename}.inc' }
	filter "files:shaders/**"
		buildmessage 'text2c.sh %{file.relpath} %{cfg.objdir}/shaders/%{file.basename}.inc'
		buildcommands { path.getabsolute('.') .. '/text2c.sh "%{file.relpath}" "%{cfg.objdir}/shaders/%{file.basename}.inc"' }
		buildoutputs { '%{cfg.objdir}/shaders/%{file.basename}.inc' }
	filter {}

	-- include common stuff
	dofile "../ud/common-proj.lua"

	exceptionhandling "Default"

	if qt then
		qt.enable()
--		qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
		qtmodules { "core", "qml", "quick", "gui" }
		qtprefix "Qt5"
		pic "on"
		configuration { "Debug or DebugOpt" }
			qtsuffix "d"
	end
