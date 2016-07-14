#!/bin/bash
set -e

echo "Update build to 'latest'..."

. build/setvars.sh


# copy build to 'latest' directory (HACK! replace with symlink...)
if [ $BUILT_TYPE == "master" ]; then

  echo "Copy build to latest/..."

  rm -rf $PACKAGE_ROOT/old
  if [ -d "$PACKAGE_ROOT/latest" ]; then
    mv $PACKAGE_ROOT/latest $PACKAGE_ROOT/old
  fi

  cp -r $PACKAGE_PATH $PACKAGE_ROOT/latest
fi
