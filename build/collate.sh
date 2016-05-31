#!/bin/bash
set -e

echo "Generate package..."

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
echo "Collating Windows package..."
WINDOWS_ROOT=$PACKAGE_PATH/windows
mkdir -p $WINDOWS_ROOT/include
mkdir -p $WINDOWS_ROOT/lib
mkdir -p $WINDOWS_ROOT/plugins
cp -R libep/public/include/* $WINDOWS_ROOT/include
cp -R $BUILD_PATH/windows_shell_release_32/lib/* $WINDOWS_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_release_64/lib/* $WINDOWS_ROOT/lib/
cp -R $BUILD_PATH/windows_shell_release_64/plugin/* $WINDOWS_ROOT/plugins/
cp $BUILD_PATH/windows_shell_release_64/bin/epshell.exe $WINDOWS_ROOT/
cp $BUILD_PATH/windows_shell_release_64/bin/epviewer.exe $WINDOWS_ROOT/
cp 3rdparty/assimp-3.1.1/lib/windows/x64/assimp-ep64.dll $WINDOWS_ROOT/
cp ud/3rdParty/sdl2/lib/x64/SDL2.dll $WINDOWS_ROOT/

# arrange ubuntu package
echo "Collating Ubuntu package..."
UBUNTU_ROOT=$PACKAGE_PATH/ubuntu/trusty
mkdir -p $UBUNTU_ROOT/include
mkdir -p $UBUNTU_ROOT/lib
mkdir -p $UBUNTU_ROOT/plugins
cp -R libep/public/include/* $UBUNTU_ROOT/include
cp -R $BUILD_PATH/ubuntu_shell_release_clang/lib/* $UBUNTU_ROOT/lib/
cp -R $BUILD_PATH/ubuntu_shell_release_clang/plugin/* $UBUNTU_ROOT/plugins/
cp $BUILD_PATH/ubuntu_shell_release_clang/bin/epshell $UBUNTU_ROOT/
cp $BUILD_PATH/ubuntu_shell_release_clang/bin/epviewer $UBUNTU_ROOT/
cp bin/amd64/libassimp-ep.so.3.1.1 $UBUNTU_ROOT/
#TODO: make .deb

# arrange centos package
echo "Collating CentOS package..."
CENTOS_ROOT=$PACKAGE_PATH/centos7
mkdir -p $CENTOS_ROOT/include
mkdir -p $CENTOS_ROOT/lib
mkdir -p $CENTOS_ROOT/plugins
cp -R libep/public/include/* $CENTOS_ROOT/include
cp -R $BUILD_PATH/ubuntu_shell_release_clang/lib/* $CENTOS_ROOT/lib/
cp -R $BUILD_PATH/ubuntu_shell_release_clang/plugin/* $CENTOS_ROOT/plugins/
cp $BUILD_PATH/ubuntu_shell_release_clang/bin/epshell $CENTOS_ROOT/
cp $BUILD_PATH/ubuntu_shell_release_clang/bin/epviewer $CENTOS_ROOT/
cp bin/amd64/libassimp-ep.so.3.1.1 $CENTOS_ROOT/
#TODO: make .rpm


# generate docs
echo "Generate docs..."
mkdir -p public/doc
cd libep
doxygen doc/Doxyfile
cd ..
cp -R public/doc $PACKAGE_PATH/docs


# generate commit log
echo "TODO: Generate commit log..."
#TODO...


# copy build to 'latest' directory (HACK! replace with symlink...)
if [ $BUILT_TYPE == "master" ]; then
  echo "Copy build to latest/..."
  rm -rf $PACKAGE_ROOT/old
  mv $PACKAGE_ROOT/latest $PACKAGE_ROOT/old
  cp -r $PACKAGE_PATH $PACKAGE_ROOT/latest
fi
