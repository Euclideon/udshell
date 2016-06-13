#!/bin/bash
set -e

date
echo

echo "Generate Windows $1 packages..."

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

set +e

date
time ./bin/dinkey/DinkeyAdd.exe "build\\dinkey\\platform.dapf /s"
errcode=$?
echo
date

if [ $errcode -ne 0 ]; then # Protecting the build failed!
	echo "DinkeyAdd.exe error: $errcode"
	exit 3
fi

set -e

echo "Dongle protection $1 successful!"

echo "Copying protected $1 binaries to $WIN_ROOT..."
cp ./epshell.exe $WIN_ROOT/
cp ./epviewer.exe $WIN_ROOT/
rm ./*.exe
echo "done"
date


# Build windows installers
echo "Creating Windows installers..."

rm -rf tempinstalldir
cp -r $WIN_ROOT tempinstalldir
if [ $1 == "win32" ]; then
  ./bin/nsis/makensis -DTARGET32 build/installer/installer.nsi
  mv build/installer/EP_Setup.exe $PACKAGE_PATH/EP_Setup_Win32.exe
elif [ $1 == "win64" ]; then
  ./bin/nsis/makensis -DTARGET64 build/installer/installer.nsi
  mv build/installer/EP_Setup.exe $PACKAGE_PATH/EP_Setup_Win64.exe
else 
  echo "Error unsupported windows target"
  exit 3
fi
rm -rf tempinstalldir

date
echo "$0 $1 complete"

