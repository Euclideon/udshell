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
	includedirs { "../udPlatform/Include" }
	includedirs { "../udPointCloud/Include" }
	includedirs { "../3rdParty/lua/" .. luaPath .. "/src"}
	includedirs { "../3rdParty/assimp-3.1.1/include" }

	if not qt then
--		includedirs { "../3rdParty/GL/freeglut/static/Include" }
		includedirs { "../3rdParty/sdl2/include" }
		includedirs { "../3rdParty/GL/glew/include" }
		defines { "GLEW_STATIC" }
	end

	filter "windows"
		includedirs { "../3rdParty/sdl2/include" }
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
	dofile "../common-proj.lua"

	exceptionhandling "Default"

	if qt then
		qt.enable()
--		qtpath "C:/dev/Qt/5.4" -- ** Expect QTDIR is set
		qtmodules { "core", "qml", "quick", "gui" }
		qtprefix "Qt5"
		configuration { "Debug or DebugOpt" }
			qtsuffix "d"
	end
