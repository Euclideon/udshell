#!/bin/bash
set -e

echo "Gather build outputs..."

. build/setvars.sh

# clean up!

echo "Cleaning intermediates..."

rm -rf $BUILD_PATH

echo "Cleaning old master builds..."

# Delete everything except the last 10 builds
ls -dt $BUILD_ROOT/Builds/master/* | tail -n +11 | xargs rm -rf
