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

. build/setvars.sh

DIST_ROOT=$PACKAGE_PATH/$dist

echo "PACKAGE_PATH: $PACKAGE_PATH"
echo "DIST_ROOT: $DIST_ROOT"

echo "Dinkey Dongle protecting binaries..."

cp $DIST_ROOT/epshell_unprotected .
cp $DIST_ROOT/epviewer_unprotected .

set +e

date
time ./bin/dinkey/DinkeyAdd.exe "build\\dinkey\\platform_linux.dapf /s"
errcode=$?
echo
date

if [ $errcode -ne 0 ]; then # Protecting the build failed!
  echo "DinkeyAdd.exe error: $errcode"
  if [ $errcode -eq 127 ]; then
    echo " - Error 127 usually means the Dinkey .o hasn't been linked to the binary!"
  fi
  exit 3
fi

set -e

echo "Dongle protection successful!"

echo "Copying protected binaries to $DIST_ROOT..."
cp ./epshell $DIST_ROOT/
cp ./epviewer $DIST_ROOT/
rm ./epshell ./epviewer ./epshell_unprotected ./epviewer_unprotected
echo "done"
date

echo "$0 complete"
