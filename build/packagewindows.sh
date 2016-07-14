#!/bin/bash
set -e

date
echo

. build/setvars.sh

echo "Generate $1 packages..."

WIN_ROOT=$PACKAGE_PATH/$1

# Build windows installers

echo "Creating Windows installers..."

rm -rf tempinstalldir
cp -r $WIN_ROOT tempinstalldir

# Use Qt's windeployqt tool to simplify deployment and packaging
$QTDIR/bin/windeployqt --release --no-quick-import --no-translations --no-angle --no-opengl-sw --no-system-d3d-compiler --no-webkit2 --dir tempinstalldir/qt --plugindir tempinstalldir/qt/plugins --libdir tempinstalldir/qt/libs tempinstalldir/epshell.exe
$QTDIR/bin/windeployqt --release --no-plugins --no-libraries --no-translations --qmldir shell/qml --dir tempinstalldir/qt/qml tempinstalldir/epshell.exe

if [ $1 == "win32" ]; then
  ./bin/nsis/makensis -DTARGET32 build/package/windows/installer.nsi
elif [ $1 == "win64" ]; then
  ./bin/nsis/makensis -DTARGET64 build/package/windows/installer.nsi
else
  echo "Error unsupported windows target"
  exit 3
fi

mv build/package/windows/epsdk.exe $PACKAGE_PATH/$PACKAGE_NAME.exe
rm -rf tempinstalldir

date
echo "$0 $1 complete"
