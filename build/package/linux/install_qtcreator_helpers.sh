#!/bin/bash

echo "Installing qtcreator plugins..."
echo

CREATOR_DIR=`which qtcreator 2>/dev/null`
if [[ $CREATOR_DIR ]]; then
  echo "Found qtcreator binary at $CREATOR_DIR, leave empty to use this location"
fi
read -p "Enter the path to the qtcreator binary: " USER_DIR

if [[ $USER_DIR ]]; then
  CREATOR_DIR=$USER_DIR
fi

if [[ ! $CREATOR_DIR =~ qtcreator$ || ! -f $CREATOR_DIR ]]; then
  echo "$CREATOR_DIR is not a valid path to a qtcreator binary!"
  exit 1
fi

if [[ "$CREATOR_DIR" =~ /usr/(local/)?(s)?bin/.* ]]; then

  if [[ "$CREATOR_DIR" =~ /usr/?(s)?bin/.* ]]; then
    # goes to /usr/share/qtcreator/...  (TODO: investigate why system installed QtCreator doesn't search /usr/local/...')
    echo "Removing old install..."
    sudo rm -rf /usr/share/qtcreator/templates/wizards/euclideon
    # legacy removal - these can be removed in a few releases
    sudo rm -rf /usr/share/qtcreator/templates/wizards/epplugin
    sudo rm -rf /usr/share/qtcreator/templates/wizards/epactivity

    echo "Copying wizard..."
    sudo mkdir -p /usr/share/qtcreator/templates/wizards/euclideon
    sudo cp -r usr/share/qtcreator/templates/wizards/* /usr/share/qtcreator/templates/wizards/euclideon/
  fi

else

  # trim 'qtcreator' (ie, the exe's filename) from the path
  CREATOR_DIR=${CREATOR_DIR%qtcreator}

  # goes to $CREATOR_DIR/../share/qtcreator/...
  echo "Removing old install..."
  rm -rf $CREATOR_DIR/../share/qtcreator/templates/wizards/euclideon
  # legacy removal - these can be removed in a few releases
  rm -rf $CREATOR_DIR/../share/qtcreator/templates/wizards/epplugin
  rm -rf $CREATOR_DIR/../share/qtcreator/templates/wizards/epactivity

  echo "Copying wizard..."
  mkdir -p $CREATOR_DIR/../share/qtcreator/templates/wizards/euclideon
  cp -r usr/share/qtcreator/templates/wizards/* $CREATOR_DIR/../share/qtcreator/templates/wizards/euclideon/

fi

echo "Done!"
