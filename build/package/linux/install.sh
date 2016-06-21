#!/bin/bash


echo "Removing old install..."

sudo rm -rf /opt/Euclideon/Shell
sudo rm -rf /usr/local/include/ep
sudo rm -f /usr/local/lib/liblibep.a
sudo rm -f /usr/local/share/applications/epshell.desktop
sudo rm -rf /usr/local/share/pixmaps/euclideon
sudo rm -f /usr/local/share/pixmaps/euclideon.png
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
sudo cp -rv usr/share/* /usr/local/share/
sudo cp -rv usr/share/pixmaps/euclideon/64x64/euclideon.png /usr/local/share/pixmaps/euclideon.png


echo "Installing SDK to /usr/local/..."

# copy the sdk
sudo mkdir -p /usr/local/include/
sudo cp -r include/ep /usr/local/include/

sudo mkdir -p /usr/local/lib/
sudo cp lib/liblibep.a /usr/local/lib/


echo "Install QtCreator project templates..."

mkdir -p ~/.config/QtProject/qtcreator
cp -rf usr/share/qtcreator ~/.config/QtProject/


echo "Creating local directory structure in ~/.local/share/Euclideon/..."

mkdir -p ~/.local/share/Euclideon/plugins


echo "Installed successfully!"

