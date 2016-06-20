#!/bin/bash
set -e

PACKAGE_ROOT=/mnt/Resources/Builds/Platform/Builds

if [ -z "$CI_BUILD_TAG" ]; then
  BUILT_TYPE=master
  BUILD_NAME=$CI_BUILD_REF
  BUILD_SHORT_NAME=${CI_BUILD_REF:0:6}
else
  BUILT_TYPE=release
  BUILD_NAME=$CI_BUILD_TAG
  BUILD_SHORT_NAME=$BUILD_NAME
fi

PACKAGE_PATH=$PACKAGE_ROOT/$BUILT_TYPE/$BUILD_NAME

DATE=`date +%y%m%d`
PACKAGE_NAME=epsdk-$1-$DATE-$BUILD_SHORT_NAME


# copy it locally for working on
echo "Formatting package..."

cp -r $PACKAGE_PATH/$1 $PACKAGE_NAME

cd $PACKAGE_NAME

rm *_unprotected

# Qt
mkdir -p Qt
cp $QTDIR/lib/libicudata.so.56.1 Qt/.
ln -s libicudata.so.56.1 Qt/libicudata.so.56
cp $QTDIR/lib/libicui18n.so.56.1 Qt/.
ln -s libicui18n.so.56.1 Qt/libicui18n.so.56
cp $QTDIR/lib/libicuuc.so.56.1 Qt/.
ln -s libicuuc.so.56.1 Qt/libicuuc.so.56

cp $QTDIR/lib/libQt5Core.so.5.6.0 Qt/.
ln -s libQt5Core.so.5.6.0 Qt/libQt5Core.so.5.6
ln -s libQt5Core.so.5.6.0 Qt/libQt5Core.so.5
ln -s libQt5Core.so.5.6.0 Qt/libQt5Core.so
cp $QTDIR/lib/libQt5Gui.so.5.6.0 Qt/.
ln -s libQt5Gui.so.5.6.0 Qt/libQt5Gui.so.5.6
ln -s libQt5Gui.so.5.6.0 Qt/libQt5Gui.so.5
ln -s libQt5Gui.so.5.6.0 Qt/libQt5Gui.so
cp $QTDIR/lib/libQt5Quick.so.5.6.0 Qt/.
ln -s libQt5Quick.so.5.6.0 Qt/libQt5Quick.so.5.6
ln -s libQt5Quick.so.5.6.0 Qt/libQt5Quick.so.5
ln -s libQt5Quick.so.5.6.0 Qt/libQt5Quick.so
cp $QTDIR/lib/libQt5Qml.so.5.6.0 Qt/.
ln -s libQt5Qml.so.5.6.0 Qt/libQt5Qml.so.5.6
ln -s libQt5Qml.so.5.6.0 Qt/libQt5Qml.so.5
ln -s libQt5Qml.so.5.6.0 Qt/libQt5Qml.so
cp $QTDIR/lib/libQt5Widgets.so.5.6.0 Qt/.
ln -s libQt5Widgets.so.5.6.0 Qt/libQt5Widgets.so.5.6
ln -s libQt5Widgets.so.5.6.0 Qt/libQt5Widgets.so.5
ln -s libQt5Widgets.so.5.6.0 Qt/libQt5Widgets.so
cp $QTDIR/lib/libQt5Network.so.5.6.0 Qt/.
ln -s libQt5Network.so.5.6.0 Qt/libQt5Network.so.5.6
ln -s libQt5Network.so.5.6.0 Qt/libQt5Network.so.5
ln -s libQt5Network.so.5.6.0 Qt/libQt5Network.so
cp $QTDIR/lib/libQt5DBus.so.5.6.0 Qt/.
ln -s libQt5DBus.so.5.6.0 Qt/libQt5DBus.so.5.6
ln -s libQt5DBus.so.5.6.0 Qt/libQt5DBus.so.5
ln -s libQt5DBus.so.5.6.0 Qt/libQt5DBus.so
cp $QTDIR/lib/libQt5XcbQpa.so.5.6.0 Qt/.
ln -s libQt5XcbQpa.so.5.6.0 Qt/libQt5XcbQpa.so.5.6
ln -s libQt5XcbQpa.so.5.6.0 Qt/libQt5XcbQpa.so.5
ln -s libQt5XcbQpa.so.5.6.0 Qt/libQt5XcbQpa.so

mkdir -p Qt/plugins
cp -r $QTDIR/plugins/platforms Qt/plugins/.
cp -r $QTDIR/plugins/xcbglintegrations Qt/plugins/.
mkdir -p Qt/qml
cp -r $QTDIR/qml/QtQuick Qt/qml/.
cp -r $QTDIR/qml/QtQuick.2 Qt/qml/.
cp -r $QTDIR/qml/Qt Qt/qml/.
cp -r $QTDIR/qml/QtQml Qt/qml/.
cp -r $QTDIR/qml/QtGraphicalEffects Qt/qml/.
cp -r $QTDIR/qml/QtMultimedia Qt/qml/.

ln -s libassimp-ep.so.3.1.1 libassimp-ep.so.3
ln -s libassimp-ep.so.3.1.1 libassimp-ep.so

chmod +x epshell*
chmod +x epviewer

chmod +x install.sh
chmod +x install_qtcreator_helpers.sh

cd ..


# make .tar.bz2
echo "Packing $PACKAGE_NAME.tar.bz2..."

tar -cvjSf $PACKAGE_NAME.tar.bz2 $PACKAGE_NAME/

cp $PACKAGE_NAME.tar.bz2 $PACKAGE_PATH


# make .deb
if [ $2 == "deb" ]; then
  echo "Generate $PACKAGE_NAME.deb..."

  # TODO
fi

# make .rpm
if [ $2 == "rpm" ]; then
  echo "Generate $PACKAGE_NAME.rpm..."

  # TODO
fi
