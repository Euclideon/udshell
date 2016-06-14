#!/bin/bash
set -e

echo "Generate Windows packages..."

PACKAGE_ROOT=//bne-fs-fs-002.euclideon.local/Resources/Builds/Platform/Builds

if [ -z "$CI_BUILD_TAG" ]; then
  BUILT_TYPE=master
  BUILD_NAME=$CI_BUILD_REF
else
  BUILT_TYPE=release
  BUILD_NAME=$CI_BUILD_TAG
fi

PACKAGE_PATH=$PACKAGE_ROOT/$BUILT_TYPE/$BUILD_NAME
WIN_ROOT=$PACKAGE_PATH/$1


echo "Dinkey Dongle protect binaries..."

# Protect 64bit build
cp $WIN_ROOT/*.exe .

./bin/dinkey/DinkeyAdd.exe "build\\dinkey\\platform.dapf /s"
if [ $? -ne 0 ]; then # Protecting the build failed!
	exit 3
fi

cp ./epshell.exe $WIN_ROOT/
cp ./epviewer.exe $WIN_ROOT/
rm ./*.exe


# Build windows installers
echo "Creating Windows installers..."

#TODO:...
