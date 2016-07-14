#!/bin/bash
set -e

if [[ $CI_BUILD_REF_NAME == "master" || -n "$CI_BUILD_TAG" ]]; then
  echo "Copying build to network..."

  . build/setvars.sh

  # affix the CI build name to the build path
  OUTPUT_PATH=$BUILD_PATH/$CI_BUILD_NAME

  mkdir -p $OUTPUT_PATH/bin
  mkdir -p $OUTPUT_PATH/lib/$PLATFORM_NAME
  mkdir -p $OUTPUT_PATH/plugin

  if [ $OSTYPE == "msys" ]; then # Windows, MingW
    cp ./public/bin/$OUTPUT_DIR/ep*.exe $OUTPUT_PATH/bin

    cp ./bin/$OUTPUT_DIR/libep.lib $OUTPUT_PATH/lib/$PLATFORM_NAME
#    cp ./bin/$OUTPUT_DIR/libdep.lib $OUTPUT_PATH/lib/$PLATFORM_NAME

    cp ./bin/$OUTPUT_DIR/*.dll $OUTPUT_PATH/plugin
  else
    cp ./public/bin/$OUTPUT_DIR/ep* $OUTPUT_PATH/bin

    cp ./bin/$OUTPUT_DIR/liblibep.a $OUTPUT_PATH/lib
#    cp ./bin/$OUTPUT_DIR/liblibdep.a $OUTPUT_PATH/lib

    cp ./bin/plugins/*.so $OUTPUT_PATH/plugin
  fi
fi
