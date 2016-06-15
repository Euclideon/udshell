#!/bin/bash
set -e

date
echo

if [ -z "$1" ]; then
    echo "Error: Need distribution name as first arg"
		exit 3
fi

dist="$1"

echo "Applying dongle protection to $dist package..."

PACKAGE_ROOT=//bne-fs-fs-002.euclideon.local/Resources/Builds/Platform/Builds

if [ -z "$CI_BUILD_TAG" ]; then
  BUILT_TYPE=master
  BUILD_NAME=$CI_BUILD_REF
else
  BUILT_TYPE=release
  BUILD_NAME=$CI_BUILD_TAG
fi

PACKAGE_PATH=$PACKAGE_ROOT/$BUILT_TYPE/$BUILD_NAME
DIST_ROOT=$PACKAGE_PATH/$dist

echo "PACKAGE_PATH: $PACKAGE_PATH"
echo "DIST_ROOT: $DIST_ROOT"

echo "Dinkey Dongle protecting binaries..."

cp $DIST_ROOT/epshell_unprotected.exe .
cp $DIST_ROOT/epviewer_unprotected.exe .

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

echo "Dongle protection successful!"

echo "Copying protected binaries to $DIST_ROOT..."
cp ./epshell.exe $DIST_ROOT/
cp ./epviewer.exe $DIST_ROOT/
rm ./epshell.exe ./epviewer.exe ./epshell_unprotected.exe ./epviewer_unprotected.exe
echo "done"
date

echo "$0 complete"
