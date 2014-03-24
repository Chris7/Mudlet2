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
# A (g)awk script to mangle the Makefile created by qmake / QtCreator FOR
# *nix operating systems so that install/uninstall actions get passed though
# sudo so they can be executed within QtCreator.
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
# Override the OWNER and GROUP variables if they are not suited to your
# requirements.
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
BEGIN {
    DEBUG=0
    if( OWNER=="" )
    {
        OWNER="root"
    }
    if( GROUP=="" )
    {
        GROUP="users"
    }
    RESULT=0
    SANITY=0
}
{
# Safety checks
    if( $1=="INSTALL_FILE" && $2=="=" )
    {
        if( $3 != "install" || $4 != "-m" || $5 != "644" || $6 != "-p" || NF !=6 )
        {
            if( DEBUG != 0 )
            {
	        print "The INSTALL_FILE Makefile command is not as anticipated, aborting." > "/dev/stderr"
                printf("1:\"%s\" 2:\"%s\" 3:\"%s\" 4:\"%s\" 5:\"%s\" 6:\"%s\" NF:%i\n", $1, $2, $3, $4, $5, $6, NF) > "/dev/stderr"
            }
            RESULT=1
            exit(1)
        }
        else
        {
            SANITY+=1
        }
        print
    }
    else if( $1=="INSTALL_PROGRAM" && $2=="=" )
    {
        if( $3 != "install" || $4 != "-m" || $5 != "755" || $6 != "-p" || NF != 6 )
        {
            if( DEBUG != 0 )
            {
	        print "The INSTALL_PROGRAM Makefile command is not as anticipated, aborting." > "/dev/stderr"
                printf("1:\"%s\" 2:\"%s\" 3:\"%s\" 4:\"%s\" 5:\"%s\" 6:\"%s\" NF:%i\n", $1, $2, $3, $4, $5, $6, NF) > "/dev/stderr"
            }
            RESULT=1
            exit(1)
        }
        else
        {
            SANITY+=2
        }
        print
    }
    else if( $1=="DEL_FILE" && $2=="=" )
    {
        if( $3 != "rm" || $4 != "-f" || NF != 4 )
        {
            if( DEBUG != 0 )
            {
                print "The DEL_FILE Makefile command is not as anticipated, aborting." > "/dev/stderr"
                printf("1:\"%s\" 2:\"%s\" 3:\"%s\" 4:\"%s\" NF:%i\n", $1, $2, $3, $4, NF) > "/dev/stderr"
            }
            RESULT=1
            exit(1)
        }
        else
        {
            SANITY+=4
        }
        print
    }
    else if( $1=="DEL_DIR" && $2=="=" )
    {
        if( $3 != "rmdir" || NF != 3 )
        {
            if( DEBUG != 0 )
            {
	        print "The DEL_DIR Makefile command is not as anticipated, aborting." > "/dev/stderr"
                printf("1:\"%s\" 2:\"%s\" 3:\"%s\" NF:%i\n", $1, $2, $3, NF) > "/dev/stderr"
            }
            RESULT=1
            exit(1)
        }
        else
        {
            SANITY+=8
        }
        print
    }
    else if( $1=="MKDIR" && $2=="=" )
    {
        if ( $3 != "mkdir" || $4 != "-p" || NF != 4 )
        {
            if( DEBUG != 0 )
            {
	        print "The MKDIR Makefile command is not as anticipated, aborting." > "/dev/stderr"
                printf("1:\"%s\" 2:\"%s\" 3:\"%s\" 4:\"%s\" NF:%i\n", $1, $2, $3, $4, NF) > "/dev/stderr"
            }
            RESULT=1
            exit(1)
        }
        else
        {
            SANITY+=16
        }
        print
    }
# By the time we see this in the Makefile this has already been used within the file
# but by checking it we can be reasonable sure we'll look for correct thing
    else if ( $1=="CHK_DIR_EXISTS=" )
    {
        if( $2!="test" || $3!="-d" || NF != 3 )
        {
            if( DEBUG != 0 )
            {
	        print "The CHK_DIR_EXISTS Makefile command is not as anticipated, aborting." > "/dev/stderr"
                printf("1:\"%s\" 2:\"%s\" 3:\"%s\" NF:%i\n", $1, $2, $3, NF) > "/dev/stderr"
            }
            RESULT=1
            exit(1)
        }
        else
        {
            SANITY+=32
        }
        print
    }
# OK Start actual work
# Reverse order of items in uninstall commmand
    else if( $1 ~ "uninstall:")
    {
        for(A=1;A<=NF;A++)
        {
            if (A==1)
            {
                printf( "%s", $A )
            }
            else
            {
                if (A==NF)
                {
                    printf(" %s\n", $A)
                }
                else
                {
                    printf(" %s", $(NF-A+1) )
                }
            }
        }
	if( DEBUG != 0 )
        {
            printf("Processed an uninstall \"%s\"...\n", $0) > "/dev/stderr"
        }
    }
# Replace all directory test/creation lines EXCEPT in .tmp* intermediate case
    else if( $1=="@test" && $2=="-d" && $3!~/^.tmp/ && $4=="||" && $5=="mkdir" && $6=="-p" && NF==7 )
    {
        printf("\t%s %s %s %s sudo -A sh -c \"install -v -o %s -g %s -d %s\"\n", $1, $2, $3, $4, OWNER, GROUP, $7)
	if( DEBUG != 0 )
        {
            printf("Processed a create directory \"%s\"...\n", $7) > "/dev/stderr"
        }
    }
# Replace INSTALL_FILE (non-executable) lines, unfortunately can't seem to
# detect tab at start of line
    else if( $1=="-$(INSTALL_FILE)" && NF==3 )
    {
        printf("\tsudo -A sh -c \"install -m 644 -p -o %s -g %s \\\"%s\\\" \\\"%s\\\" \"\n", OWNER, GROUP, $2, $3)
	if( DEBUG !=0 )
        {
            printf("Processed an install file \"%s\"...\n", $3) > "/dev/stderr"
        }
    }
# Replace INSTALL_PROGRAM (executable) lines, both source and destination are
# quoted which we have to undo to escape them
    else if( $1=="-$(INSTALL_PROGRAM)" && NF==3 )
    {
        gsub("\"", "", $2)
        gsub("\"", "", $3)
        printf("\tsudo -A sh -c \"install -m 655 -p -o %s -g %s \\\"%s\\\" \\\"%s\\\" \"\n", OWNER, GROUP, $2, $3)
	if( DEBUG !=0 )
        {
            printf("Processed an install program \"%s\"...\n", $3) > "/dev/stderr"
        }
    }
# Replace DEL_FILE (executable or non) lines ONLY for INSTALLION cases, the
# executable case uses QMAKE_TARGET variable and quotes the target which we
# have to undo to escape it
    else if( $1=="-$(DEL_FILE)" && NF==2 && $2 ~ /^\"\$\(INSTALL_ROOT\)/ && $2 ~ /\$\(QMAKE_TARGET\)\"$/ )
    {
        gsub("\"", "", $2)
        printf("\tsudo -A sh -c \"rm \\\"%s\\\"\"\n", $2)
        if( DEBUG !=0 )
        {
            printf("Processed a delete program \"%s\"...\n", $2) > "/dev/stderr"
        }
    }
    else if( $1=="-$(DEL_FILE)" && NF==3 && $2=="-r" && $3 ~ /^\$\(INSTALL_ROOT\)/ )
    {
        printf("\tsudo -A sh -c \"rm \\\"%s\\\"\"\n", $3)
	if( DEBUG !=0 )
        {
            printf("Processed a delete file \"%s\"...\n", $3) > "/dev/stderr"
        }
    }
    else if( $1== "-$(DEL_DIR)" && NF==2 )
    {
        printf("\tsudo -A sh -c \"rmdir -v --ignore-fail-on-non-empty \\\"%s\\\"\"\n", $2)
	if( DEBUG !=0 )
        {
            printf("Processed a delete directory \"%s\"...\n", $2) > "/dev/stderr"
        }
    }
# Default: pass line through unchanged
    else
    {
        print
    }
}
END {
    if( SANITY==63 && RESULT==0 )
    {
        if( DEBUG!=0 )
        {
            print "Processed entire file, it should be usable!" > "/dev/stderr"
        }
    }
    else if( SANITY!=63 )
    {
        printf("There was a problem processing the Makefile, one or more expected elements were not found!\nYou may wish to report the following: \"SANITY value was only:%i instead of expected 63\"\n", SANITY) > "/dev/stderr"
        exit(1)
    }
    else
    {
	if( DEBUG==0)
        {
            print "There was a problem processing the Makefile, it is not usable for installation!\nChanging the value of DEBUG to 1 in the BEGIN block of rewriteMakefile.awk\nin the Mudlet src directory and repeating the qmake and deploy stages will give more information." > "/dev/stderr"
        }
        else
        {
            print "There was a problem processing the Makefile, it is not usable for installation!" > "/dev/stderr"
        }
    }
}
