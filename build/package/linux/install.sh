#!/bin/bash


INSTALL_PATH=/usr/local


echo "Removing old install..."

sudo rm -rf /opt/Euclideon/Shell
sudo rm -rf $INSTALL_PATH/include/ep
sudo rm -f $INSTALL_PATH/lib/liblibep.a
sudo rm -f $INSTALL_PATH/share/applications/epshell.desktop
sudo rm -f $INSTALL_PATH/share/applications/epviewer.desktop
sudo rm -rf $INSTALL_PATH/share/pixmaps/euclideon
sudo rm -f $INSTALL_PATH/share/pixmaps/euclideon.png
sudo rm -f $INSTALL_PATH/share/mime/packages/euclideon.xml
sudo rm -rf ~/.config/QtProject/qtcreator/templates/wizards/euclideon
sudo rm -rf $INSTALL_PATH/share/Euclideon

# legacy removal - this can be removed from the install script in a few releases
sudo rm -rf ~/.config/QtProject/qtcreator/templates/wizards/epplugin
sudo rm -rf ~/.config/QtProject/qtcreator/templates/wizards/epactivity


# if we just want to uninstall...
if [[ $1 == "--remove" || $1 == "-r" ]]; then
  exit 0
fi


echo "Installing Shell to /opt/Euclideon/Shell/..."

# make the directory structure
sudo mkdir -p /opt/Euclideon/Shell/plugins

# copy binaries
sudo cp -v epshell* /opt/Euclideon/Shell/
sudo cp -v epviewer /opt/Euclideon/Shell/

# copy Qt
sudo cp -v qt.conf /opt/Euclideon/Shell/
sudo cp -dr --preserve=links Qt /opt/Euclideon/Shell/

# copy libs
sudo cp -dv --preserve=links libassimp-ep.so* /opt/Euclideon/Shell/

# copy plugins
sudo cp -v plugins/libviewer.so /opt/Euclideon/Shell/plugins/

# copy system data
sudo cp -rv usr/share/* $INSTALL_PATH/share/
sudo cp -rv usr/share/pixmaps/euclideon/64x64/euclideon.png $INSTALL_PATH/share/pixmaps/euclideon.png


echo "Installing SDK to $INSTALL_PATH/..."

# copy the sdk
sudo mkdir -p $INSTALL_PATH/include/
sudo cp -r include/ep $INSTALL_PATH/include/

sudo mkdir -p $INSTALL_PATH/lib/
sudo cp lib/liblibep.a $INSTALL_PATH/lib/


echo "Install QtCreator project templates..."

mkdir -p ~/.config/QtProject/qtcreator
cp -rf usr/share/qtcreator ~/.config/QtProject/

echo "Install QtCreator debug helpers..."
sudo mkdir -p $INSTALL_PATH/share/Euclideon/debugger
sudo cp -rf debugger/* $INSTALL_PATH/share/Euclideon/debugger/
cat $INSTALL_PATH/share/Euclideon/debugger/README

echo "Creating local directory structure in ~/.local/share/Euclideon/..."

mkdir -p ~/.local/share/Euclideon/plugins


echo "Registering mime-types..."

sudo xdg-mime install --novendor --mode system $INSTALL_PATH/share/mime/packages/euclideon.xml
sudo xdg-mime default epshell.desktop application/epshell
sudo xdg-mime default epviewer.desktop application/uds


echo "Installed successfully!"

