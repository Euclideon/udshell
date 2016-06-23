#!/bin/bash
set -e

echo "Gather build outputs..."

BUILD_ROOT=/mnt/Resources/Builds/Platform

if [ -z "$CI_BUILD_TAG" ]; then
  BUILT_TYPE=master
  BUILD_NAME=$CI_BUILD_REF
else
  BUILT_TYPE=release
  BUILD_NAME=$CI_BUILD_TAG
fi

BUILD_PATH=$BUILD_ROOT/Intermediate/$BUILT_TYPE/$BUILD_NAME


# clean up!

echo "Cleaning intermediates..."

rm -rf $BUILD_PATH

echo "Cleaning old master builds..."

# Delete everything except the last 10 builds
ls -dt $BUILD_ROOT/Builds/master/* | tail -n +11 | xargs rm -rf