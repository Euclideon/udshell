#!/bin/bash
set -e

date
echo

echo "Generate $1 packages..."

PACKAGE_ROOT=//bne-fs-fs-002.euclideon.local/Resources/Builds/Platform/Builds

if [ -z "$CI_BUILD_TAG" ]; then
  BUILT_TYPE=master
  BUILD_NAME=$CI_BUILD_REF
  BUILD_SHORT_NAME=${CI_BUILD_REF:0:6}
else
  BUILT_TYPE=release
  BUILD_NAME=$CI_BUILD_TAG
  BUILD_SHORT_NAME=$BUILD_NAME
fi

PACKAGE_PATH=$PACKAGE_ROOT/$BUILT_TYPE/$BUILD_NAME
WIN_ROOT=$PACKAGE_PATH/$1

DATE=`date +%y%m%d`
PACKAGE_NAME=epsdk-$1-$DATE-$BUILD_SHORT_NAME


# Build windows installers

echo "Creating Windows installers..."

rm -rf tempinstalldir
cp -r $WIN_ROOT tempinstalldir
if [ $1 == "win32" ]; then
  ./bin/nsis/makensis -DTARGET32 build/package/windows/installer.nsi
elif [ $1 == "win64" ]; then
  ./bin/nsis/makensis -DTARGET64 build/package/windows/installer.nsi
else
  echo "Error unsupported windows target"
  exit 3
fi

mv build/package/windows/epsdk.exe $PACKAGE_PATH/$PACKAGE_NAME.exe
rm -rf tempinstalldir

date
echo "$0 $1 complete"
