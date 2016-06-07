#!/bin/bash
set -e

if [[ $CI_BUILD_REF_NAME == "master" || -n "$CI_BUILD_TAG" ]]; then
  echo "Copying build to network..."

  if [ $OSTYPE == "msys" ]; then # Windows, MingW
    if [ $3 == "Win32" ]; then
      PLATFORM_NAME=x86
    else
      PLATFORM_NAME=$3
    fi
    BUILD_ROOT=//bne-fs-fs-002.euclideon.local/Resources/Builds/Platform/Intermediate
  else
    BUILD_ROOT=/mnt/Resources/Builds/Platform/Intermediate
  fi

  OUTPUT_DIR=$2_$PLATFORM_NAME
  if [ -z "$CI_BUILD_TAG" ]; then
    BUILT_TYPE=master
    BUILD_NAME=$CI_BUILD_REF
  else
    BUILT_TYPE=release
    BUILD_NAME=$CI_BUILD_TAG
  fi

  BUILD_PATH=$BUILD_ROOT/$BUILT_TYPE/$BUILD_NAME/$CI_BUILD_NAME

  mkdir -p $BUILD_PATH/bin
  mkdir -p $BUILD_PATH/lib/$PLATFORM_NAME
  mkdir -p $BUILD_PATH/plugin

  if [ $OSTYPE == "msys" ]; then # Windows, MingW
    cp ./public/bin/$OUTPUT_DIR/ep*.exe $BUILD_PATH/bin

    cp ./bin/$OUTPUT_DIR/libep.lib $BUILD_PATH/lib/$PLATFORM_NAME
#    cp ./bin/$OUTPUT_DIR/libdep.lib $BUILD_PATH/lib/$PLATFORM_NAME

    cp ./bin/$OUTPUT_DIR/*.dll $BUILD_PATH/plugin
  else
    cp ./public/bin/$OUTPUT_DIR/ep* $BUILD_PATH/bin

    cp ./bin/$OUTPUT_DIR/liblibep.a $BUILD_PATH/lib
#    cp ./bin/$OUTPUT_DIR/liblibdep.a $BUILD_PATH/lib

    cp ./bin/plugins/*.so $BUILD_PATH/plugin
  fi
fi
