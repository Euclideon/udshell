#!/bin/bash
set -e

echo "Update build to 'latest'..."

PACKAGE_ROOT=/mnt/Resources/Builds/Platform/Builds

if [ -z "$CI_BUILD_TAG" ]; then
  BUILT_TYPE=master
  BUILD_NAME=$CI_BUILD_REF
else
  BUILT_TYPE=release
  BUILD_NAME=$CI_BUILD_TAG
fi

PACKAGE_PATH=$PACKAGE_ROOT/$BUILT_TYPE/$BUILD_NAME


# copy build to 'latest' directory (HACK! replace with symlink...)
if [ $BUILT_TYPE == "master" ]; then

  echo "Copy build to latest/..."

  rm -rf $PACKAGE_ROOT/old
  if [ -d "$PACKAGE_ROOT/latest" ]; then
    mv $PACKAGE_ROOT/latest $PACKAGE_ROOT/old
  fi

  cp -r $PACKAGE_PATH $PACKAGE_ROOT/latest
fi
