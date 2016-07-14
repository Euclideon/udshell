#!/bin/bash
set -e

. build/setvars.sh


# generate docs
echo "Generate docs..."

mkdir -p public/doc
cd libep
doxygen doc/Doxyfile
cd ..


echo "Copying build outputs..."

mkdir -p $PACKAGE_PATH
cp -R public/doc $PACKAGE_PATH/docs
