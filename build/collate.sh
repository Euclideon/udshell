#!/bin/bash
set -e

echo "Gather build outputs..."

. build/setvars.sh

# collate outputs into proper locations...

# arrange windows package
echo "Collating Win64 package..."
mkdir -p $WIN64_ROOT/include
mkdir -p $WIN64_ROOT/lib
mkdir -p $WIN64_ROOT/plugins
mkdir -p $WIN64_ROOT/qtcreator
cp -R libep/public/include/* $WIN64_ROOT/include
cp -R $BUILD_PATH/windows_shell_releasedev_32/lib/* $WIN64_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_releasedev_64/lib/* $WIN64_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_releasedev_64/plugin/* $WIN64_ROOT/plugins/
cp $BUILD_PATH/windows_shell_releasedev_64/bin/epshell.exe $WIN64_ROOT/epshell_unprotected.exe
cp $BUILD_PATH/windows_shell_releasedev_64/bin/epviewer.exe $WIN64_ROOT/epviewer_unprotected.exe
cp 3rdparty/assimp-3.1.1/lib/windows/x64/assimp-ep64.dll $WIN64_ROOT/
cp ud/3rdParty/sdl2/lib/x64/SDL2.dll $WIN64_ROOT/
cp build/package/windows/qt.conf $WIN64_ROOT/
cp $PACKAGE_PATH/docs/libep-docs.chm $WIN64_ROOT/
cp -R build/package/qtcreator/wizards/* $WIN64_ROOT/qtcreator/

# arrange windows package
echo "Collating Win32 package..."
mkdir -p $WIN32_ROOT/include
mkdir -p $WIN32_ROOT/lib
mkdir -p $WIN32_ROOT/plugins
mkdir -p $WIN32_ROOT/qtcreator
cp -R libep/public/include/* $WIN32_ROOT/include
cp -R $BUILD_PATH/windows_shell_releasedev_32/lib/* $WIN32_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_releasedev_64/lib/* $WIN32_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_releasedev_32/plugin/* $WIN32_ROOT/plugins/
cp $BUILD_PATH/windows_shell_releasedev_32/bin/epshell.exe $WIN32_ROOT/epshell_unprotected.exe
cp $BUILD_PATH/windows_shell_releasedev_32/bin/epviewer.exe $WIN32_ROOT/epviewer_unprotected.exe
cp 3rdparty/assimp-3.1.1/lib/windows/x32/assimp-ep32.dll $WIN32_ROOT/
cp ud/3rdParty/sdl2/lib/x86/SDL2.dll $WIN32_ROOT/
cp build/package/windows/qt.conf $WIN32_ROOT/
cp $PACKAGE_PATH/docs/libep-docs.chm $WIN32_ROOT/
cp -R build/package/qtcreator/wizards/* $WIN32_ROOT/qtcreator/

# arrange xenial package
echo "Collating Ubuntu xenial package..."
mkdir -p $UBUNTU_ROOT/include
mkdir -p $UBUNTU_ROOT/lib
mkdir -p $UBUNTU_ROOT/plugins
mkdir -p $UBUNTU_ROOT/debugger
cp -R libep/public/include/* $UBUNTU_ROOT/include
cp -R $BUILD_PATH/ubuntu_shell_releasedev_clang/lib/* $UBUNTU_ROOT/lib/
cp -R $BUILD_PATH/ubuntu_shell_releasedev_clang/plugin/* $UBUNTU_ROOT/plugins/
cp $BUILD_PATH/ubuntu_shell_releasedev_clang/bin/epshell $UBUNTU_ROOT/epshell_unprotected
cp $BUILD_PATH/ubuntu_shell_releasedev_clang/bin/epviewer $UBUNTU_ROOT/epviewer_unprotected
cp bin/amd64/libassimp-ep.so.3.1.1 $UBUNTU_ROOT/
cp build/package/linux/install.sh $UBUNTU_ROOT/
cp build/package/linux/epshell.sh $UBUNTU_ROOT/
cp build/package/linux/epshell_debug.sh $UBUNTU_ROOT/
cp build/package/linux/qt.conf $UBUNTU_ROOT/
cp -R build/package/qtcreator/debugger/* $UBUNTU_ROOT/debugger/
cp -R build/package/linux/usr $UBUNTU_ROOT/
cp $PACKAGE_PATH/docs/libep-docs.chm $UBUNTU_ROOT/

mkdir -p $UBUNTU_ROOT/usr/share/qtcreator/templates/wizards/euclideon
cp -R build/package/qtcreator/wizards/* $UBUNTU_ROOT/usr/share/qtcreator/templates/wizards/euclideon

strip $UBUNTU_ROOT/epshell_unprotected
strip $UBUNTU_ROOT/epviewer_unprotected
strip $UBUNTU_ROOT/plugins/*

# arrange centos package
echo "Collating CentOS7 package..."
mkdir -p $CENTOS_ROOT/include
mkdir -p $CENTOS_ROOT/lib
mkdir -p $CENTOS_ROOT/plugins
mkdir -p $CENTOS_ROOT/debugger
cp -R libep/public/include/* $CENTOS_ROOT/include
cp -R $BUILD_PATH/rpm_shell_releasedev_clang/lib/* $CENTOS_ROOT/lib/
cp -R $BUILD_PATH/rpm_shell_releasedev_clang/plugin/* $CENTOS_ROOT/plugins/
cp $BUILD_PATH/rpm_shell_releasedev_clang/bin/epshell $CENTOS_ROOT/epshell_unprotected
cp $BUILD_PATH/rpm_shell_releasedev_clang/bin/epviewer $CENTOS_ROOT/epviewer_unprotected
cp bin/amd64/libassimp-ep.so.3.1.1 $CENTOS_ROOT/
cp build/package/linux/install.sh $CENTOS_ROOT/
cp build/package/linux/epshell.sh $CENTOS_ROOT/
cp build/package/linux/epshell_debug.sh $CENTOS_ROOT/
cp build/package/linux/qt.conf $CENTOS_ROOT/
cp -R build/package/qtcreator/debugger/* $CENTOS_ROOT/debugger/
cp -R build/package/linux/usr $CENTOS_ROOT/
cp $PACKAGE_PATH/docs/libep-docs.chm $CENTOS_ROOT/

mkdir -p $CENTOS_ROOT/usr/share/qtcreator/templates/wizards/euclideon
cp -R build/package/qtcreator/wizards/* $CENTOS_ROOT/usr/share/qtcreator/templates/wizards/euclideon

strip $CENTOS_ROOT/epshell_unprotected
strip $CENTOS_ROOT/epviewer_unprotected
strip $CENTOS_ROOT/plugins/*


# generate commit log
echo "Generating commit log..."
build/genchangelog.d
cp build/CHANGELOG $UBUNTU_ROOT/.
cp build/CHANGELOG $CENTOS_ROOT/.

# fix the line endings for windows (since we're running the script on linux)
sed -i "s/$/`echo -e \\\r`/" build/CHANGELOG
cp build/CHANGELOG $WIN64_ROOT/changelog.txt
cp build/CHANGELOG $WIN32_ROOT/changelog.txt
