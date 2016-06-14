#!/bin/bash


echo "Removing old install..."

sudo rm -rf /opt/Euclideon/Shell
sudo rm -rf /usr/local/include/ep
sudo rm -f /usr/local/lib/liblibep.a
sudo rm -f /usr/share/applications/epshell.desktop
sudo rm -rf /usr/share/pixmaps/euclideon
sudo rm -f /usr/share/pixmaps/euclideon.png


echo "Installing Shell to /opt/Euclideon/Shell/..."

# make the directory structure
sudo mkdir -p /opt/Euclideon/Shell/plugins

# copy binaries
sudo cp -v epshell /opt/Euclideon/Shell/
sudo cp -v epviewer /opt/Euclideon/Shell/

# copy libs
sudo cp -v libassimp-ep.so.3.1.1 /opt/Euclideon/Shell/
sudo ln -s libassimp-ep.so.3.1.1 /opt/Euclideon/Shell/libassimp-ep.so.3
sudo ln -s libassimp-ep.so.3.1.1 /opt/Euclideon/Shell/libassimp-ep.so

# copy plugins
sudo cp -v plugins/libviewer.so /opt/Euclideon/Shell/plugins/

# copy system data
sudo cp -rv usr/share/* /usr/share/
sudo cp -rv usr/share/pixmaps/euclideon/48x48/euclideon.png /usr/share/pixmaps/euclideon.png


echo "Installing SDK to /usr/local/..."

# copy the sdk
sudo mkdir -p /usr/local/include/
sudo cp -r include/ep /usr/local/include/

sudo mkdir -p /usr/local/lib/
sudo cp lib/liblibep.a /usr/local/lib/


echo "Creating local directory structure in ~/.local/share/Euclideon/..."

mkdir -p ~/.local/share/Euclideon/plugins


echo "Installed successfully!"

