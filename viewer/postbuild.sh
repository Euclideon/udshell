#!/bin/bash
if [ $1 = "x64" ]; then
  cp ../ud/3rdParty/sdl2/lib/x64/SDL2.dll ./
  cp ../3rdparty/assimp-3.1.1/lib/windows/x64/assimp-ep64.dll ./
elif [ $1 = "x86" ]; then
  cp ../ud/3rdParty/sdl2/lib/x86/SDL2.dll ./
  cp ../3rdparty/assimp-3.1.1/lib/windows/x32/assimp-ep32.dll ./
fi
