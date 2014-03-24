#! /bin/sh
#**************************************************************************
#*   Copyright (C) 2014 by Stephen Lyons                                  *
#*   email: slysven@virginmedia.com                                       *
#*   In memoria: John Charles Lyons 1941-2013                             *
#*                                                                        *
#*   This program is free software; you can redistribute it and/or modify *
#*   it under the terms of the GNU General Public License as published by *
#*   the Free Software Foundation; either version 2 of the License, or    *
#*   (at your option) any later version.                                  *
#*                                                                        *
#*   This program is distributed in the hope that it will be useful,      *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of       *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
#*   GNU General Public License for more details.                         *
#*                                                                        *
#*   You should have received a copy of the GNU General Public License    *
#*   along with this program; if not, write to the                        *
#*   Free Software Foundation, Inc.,                                      *
#*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.            *
#**************************************************************************
#
# This uses a(g)awk script to mangle the Makefile created by qmake / QtCreator
# FOR *nix operating systems so that install/uninstall actions get passed
# though sudo so they can be executed within QtCreator.
#
# THIS WILL FAIL - POSSIBLY QUITE BADLY IF PATHS OR FILENAMES CONTAIN SPACES!!!
#
# As well as sudo and awk you will need ssh-askpass or an equivalent
# utility to enable entry of the password required to elevate priviledges in
# a GUI environment (the -A option to sudo causes it to make such password
# requests via a program specifed by a "Path askpass=/path/to/program"
# line in /etc/sudo.conf which may be overridden by an SUDO_ASKPASS
# environmental variable containing such a /path/to/program.)
#
# Override the OWNER and GROUP variables at the top of the awk script
# if they are not suited to your requirements.
#
# To setup: in the QtCreator IDE "Projects" area, select the "Build & Run" tab
# then select the "Run" subtab for the kit you are using.
# Add a "Custom Process Step" in the QtCreator IDE projects area:
#           Command: ./rewriteMakefile.sh
#         Arguments:
# Working directory: %{buildDir}
#
# Then ensure that there is a "Make step" following with the Make argument
# set to "install"
#
# You may also wish to change to the "Build" subtab for the same kit and:
# Add a (Make) Clean Step:
#  Override /usr/bin/make:
#          Make arguments: uninstall
#
# This should be BEFORE a similiar step where the Make argument is: "clean"
# as the latter is likely to remove the Makefile that we need!
#
# To use:
# After qmake has been "run" and the project has been "built" use the
# "deploy" action to install the project in the directories specified
# by the "install" sets (group of files in qmake variables with .path &
# .files - and possibly .extra or .command)
#
# To uninstall the project you may select a "Build" menu "clean" option
# which will then remove the installed elements before removing the
# locally built code files in the build area - for safety you may wish
# NOT to make any entries in the Clean area to avoid loss of code until
# you actually need to...
#
# Note: if this is file in the build directory and NOT the source (and you are
# shadow building) this file will be overwritten the next time that
# qmake is run.
#
# Move Makefile to a backup for debugging comparasion...!
mv ./Makefile ./Makefile.orig
echo "Backing up original Makefile"

echo "Processing Makefile"
awk -f ./rewriteMakefile.awk ./Makefile.orig >./Makefile

# Clear user's sudo credentials so a password will be requested the next time
# it is used, hopefully the first usage with the rewritten Makefile...
sudo -k

