#!/bin/bash
set -e

date
echo

echo "Generate $1 packages..."

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


# Build windows installers

echo "Creating Windows installers..."

rm -rf tempinstalldir
cp -r $WIN_ROOT tempinstalldir
if [ $1 == "win32" ]; then
  ./bin/nsis/makensis -DTARGET32 build/package/windows/installer.nsi
  mv build/package/windows/EP_Setup.exe $PACKAGE_PATH/EP_Setup_Win32.exe
elif [ $1 == "win64" ]; then
  ./bin/nsis/makensis -DTARGET64 build/package/windows/installer.nsi
  mv build/package/windows/EP_Setup.exe $PACKAGE_PATH/EP_Setup_Win64.exe
else
  echo "Error unsupported windows target"
  exit 3
fi
rm -rf tempinstalldir

date
echo "$0 $1 complete"
