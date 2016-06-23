#!/bin/bash
set -e

if [ ! -f "build/makeinstaller.sh" ]; then
  echo "This script must be run from the root of the epshell repository"
  exit -1
fi

echo "Creating Windows installers..."

#Point this to the desired package root to run this script locally
PACKAGE_PATH="//bne-fs-fs-002.euclideon.local/Resources/Builds/Platform/Builds/latest"

WIN32_ROOT=$PACKAGE_PATH/win32
WIN64_ROOT=$PACKAGE_PATH/win64

rm -rf tempinstalldir
cp -r $WIN32_ROOT tempinstalldir
./bin/nsis/makensis -DTARGET32 build/package/windows/installer.nsi 
mv build/package/windows/epsdk.exe epsdk-win32.exe

rm -rf tempinstalldir
cp -r $WIN64_ROOT tempinstalldir
./bin/nsis/makensis -DTARGET64 build/package/windows/installer.nsi
mv build/package/windows/epsdk.exe epsdk-win64.exe

rm -rf tempinstalldir
rm -f build/package/windows/*.exe
