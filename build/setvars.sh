if [ $OSTYPE == "msys" ]; then # Windows, MingW
  if [ $3 == "win32" ]; then
    PLATFORM_NAME=x86
  else
    PLATFORM_NAME=$3
  fi
  BUILD_ROOT=//bne-fs-fs-002.euclideon.local/Resources/Builds/Platform/Intermediate
  PACKAGE_ROOT=//bne-fs-fs-002.euclideon.local/Resources/Builds/Platform/Builds
else
  BUILD_ROOT=/mnt/Resources/Builds/Platform/Intermediate
  PACKAGE_ROOT=/mnt/Resources/Builds/Platform/Builds
fi

OUTPUT_DIR=$2_$PLATFORM_NAME

if [ -z "$CI_BUILD_TAG" ]; then
  BUILT_TYPE=$CI_BUILD_REF_NAME
  BUILD_NAME=$CI_BUILD_REF
  DATE=`date +%y%m%d`
  BUILD_SHORT_NAME=$DATE-${CI_BUILD_REF:0:6}
else
  BUILT_TYPE=release
  BUILD_NAME=$CI_BUILD_TAG
  BUILD_SHORT_NAME=$BUILD_NAME
fi

BUILD_PATH=$BUILD_ROOT/$BUILT_TYPE/$BUILD_NAME
PACKAGE_PATH=$PACKAGE_ROOT/$BUILT_TYPE/$BUILD_NAME

PACKAGE_NAME=epsdk-$1-$BUILD_SHORT_NAME

WIN32_ROOT=$PACKAGE_PATH/win32
WIN64_ROOT=$PACKAGE_PATH/win64
UBUNTU_ROOT=$PACKAGE_PATH/xenial
CENTOS_ROOT=$PACKAGE_PATH/centos7
