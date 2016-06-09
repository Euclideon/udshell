#!/bin/bash
set -e

PACKAGE_ROOT=//bne-fs-fs-002.euclideon.local/Resources/Builds/Platform/Builds

if [ -z "$CI_BUILD_TAG" ]; then
  BUILT_TYPE=master
  BUILD_NAME=$CI_BUILD_REF
else
  BUILT_TYPE=release
  BUILD_NAME=$CI_BUILD_TAG
fi

PACKAGE_PATH=$PACKAGE_ROOT/$BUILT_TYPE/$BUILD_NAME

# generate docs
echo "Generate docs..."

mkdir -p public/doc
cd libep
doxygen doc/Doxyfile
cd ..


echo "Copying build outputs..."

mkdir -p $PACKAGE_PATH
cp -R public/doc $PACKAGE_PATH/docs
