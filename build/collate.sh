#!/bin/bash
set -e

echo "Gather build outputs..."

BUILD_ROOT=/mnt/Resources/Builds/Platform/Intermediate
PACKAGE_ROOT=/mnt/Resources/Builds/Platform/Builds

if [ -z "$CI_BUILD_TAG" ]; then
  BUILT_TYPE=master
  BUILD_NAME=$CI_BUILD_REF
else
  BUILT_TYPE=release
  BUILD_NAME=$CI_BUILD_TAG
fi

BUILD_PATH=$BUILD_ROOT/$BUILT_TYPE/$BUILD_NAME
PACKAGE_PATH=$PACKAGE_ROOT/$BUILT_TYPE/$BUILD_NAME


# collate outputs into proper locations...

# arrange windows package
echo "Collating Win64 package..."
WIN64_ROOT=$PACKAGE_PATH/win64
mkdir -p $WIN64_ROOT/include
mkdir -p $WIN64_ROOT/lib
mkdir -p $WIN64_ROOT/plugins
cp -R libep/public/include/* $WIN64_ROOT/include
cp -R $BUILD_PATH/windows_shell_release_32/lib/* $WIN64_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_release_64/lib/* $WIN64_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_release_64/plugin/* $WIN64_ROOT/plugins/
cp $BUILD_PATH/windows_shell_release_64/bin/epshell.exe $WIN64_ROOT/epshell_unprotected.exe
cp $BUILD_PATH/windows_shell_release_64/bin/epviewer.exe $WIN64_ROOT/epviewer_unprotected.exe
cp 3rdparty/assimp-3.1.1/lib/windows/x64/assimp-ep64.dll $WIN64_ROOT/
cp ud/3rdParty/sdl2/lib/x64/SDL2.dll $WIN64_ROOT/
cp $PACKAGE_PATH/docs/libep-docs.chm $WIN64_ROOT/

# arrange windows package
echo "Collating Win32 package..."
WIN32_ROOT=$PACKAGE_PATH/win32
mkdir -p $WIN32_ROOT/include
mkdir -p $WIN32_ROOT/lib
mkdir -p $WIN32_ROOT/plugins
cp -R libep/public/include/* $WIN32_ROOT/include
cp -R $BUILD_PATH/windows_shell_release_32/lib/* $WIN32_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_release_64/lib/* $WIN32_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_release_32/plugin/* $WIN32_ROOT/plugins/
cp $BUILD_PATH/windows_shell_release_32/bin/epshell.exe $WIN32_ROOT/epshell_unprotected.exe
cp $BUILD_PATH/windows_shell_release_32/bin/epviewer.exe $WIN32_ROOT/epviewer_unprotected.exe
cp 3rdparty/assimp-3.1.1/lib/windows/x32/assimp-ep32.dll $WIN32_ROOT/
cp ud/3rdParty/sdl2/lib/x86/SDL2.dll $WIN32_ROOT/
cp $PACKAGE_PATH/docs/libep-docs.chm $WIN32_ROOT/

# arrange trusty package
echo "Collating Ubuntu trusty package..."
UBUNTU_ROOT=$PACKAGE_PATH/trusty
mkdir -p $UBUNTU_ROOT/include
mkdir -p $UBUNTU_ROOT/lib
mkdir -p $UBUNTU_ROOT/plugins
cp -R libep/public/include/* $UBUNTU_ROOT/include
cp -R $BUILD_PATH/ubuntu_shell_release_clang/lib/* $UBUNTU_ROOT/lib/
cp -R $BUILD_PATH/ubuntu_shell_release_clang/plugin/* $UBUNTU_ROOT/plugins/
cp $BUILD_PATH/ubuntu_shell_release_clang/bin/epshell $UBUNTU_ROOT/
cp $BUILD_PATH/ubuntu_shell_release_clang/bin/epviewer $UBUNTU_ROOT/
cp bin/amd64/libassimp-ep.so.3.1.1 $UBUNTU_ROOT/
cp build/package/linux/install.sh $UBUNTU_ROOT/
cp -R build/package/linux/usr $UBUNTU_ROOT/
cp $PACKAGE_PATH/docs/libep-docs.chm $UBUNTU_ROOT/

strip $UBUNTU_ROOT/epshell
strip $UBUNTU_ROOT/epviewer
strip $UBUNTU_ROOT/plugins/*

# arrange centos package
echo "Collating CentOS7 package..."
CENTOS_ROOT=$PACKAGE_PATH/centos7
mkdir -p $CENTOS_ROOT/include
mkdir -p $CENTOS_ROOT/lib
mkdir -p $CENTOS_ROOT/plugins
cp -R libep/public/include/* $CENTOS_ROOT/include
cp -R $BUILD_PATH/rpm_shell_release_clang/lib/* $CENTOS_ROOT/lib/
cp -R $BUILD_PATH/rpm_shell_release_clang/plugin/* $CENTOS_ROOT/plugins/
cp $BUILD_PATH/rpm_shell_release_clang/bin/epshell $CENTOS_ROOT/
cp $BUILD_PATH/rpm_shell_release_clang/bin/epviewer $CENTOS_ROOT/
cp bin/amd64/libassimp-ep.so.3.1.1 $CENTOS_ROOT/
cp build/package/linux/install.sh $CENTOS_ROOT/
cp -R build/package/linux/usr $CENTOS_ROOT/
cp $PACKAGE_PATH/docs/libep-docs.chm $CENTOS_ROOT/

strip $CENTOS_ROOT/epshell
strip $CENTOS_ROOT/epviewer
strip $CENTOS_ROOT/plugins/*


# generate commit log
echo "TODO: Generate commit log..."
#TODO...
