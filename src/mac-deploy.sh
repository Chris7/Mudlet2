#!/bin/bash
# This script needs to be run as root (http://blog.inventic.eu/2012/08/how-to-deploy-qt-application-on-macos-part-ii/)
# so if it's not working, do sudo chmod +s mac-deploy.sh
#
# This script needs to be run from inside the src/ folder.

echo Copying Qt and dependent libraries...
export PATH=$PATH:$HOME/Qt/5.1.1/clang_64/bin
echo $PATH
sudo macdeployqt ./mudlet.app -dmg
