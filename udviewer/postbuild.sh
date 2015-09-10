#!/bin/bash
if [ $1 = "x64" ]; then
  cp ../ud/3rdParty/sdl2/lib/x64/SDL2.dll ./
  cp ../bin/win64/assimp.dll ./
elif [ $1 = "x86" ]; then
  cp ../ud/3rdParty/sdl2/lib/x86/SDL2.dll ./
  cp ../bin/win32/assimp.dll ./
fi 
