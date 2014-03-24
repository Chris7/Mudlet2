QMAKE_CXXFLAGS_RELEASE += -O3 -Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-unused-parameter
QMAKE_CXXFLAGS_DEBUG += -g -Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-unused-parameter
#MOC_DIR = ./tmp
#OBJECTS_DIR = ./tmp
QT += network opengl uitools multimedia
DEPENDPATH += .
INCLUDEPATH += .
LIBLUA = -llua5.1
#!exists(/usr/lib/x86_64-linux-gnu/liblua5.1.a):LIBLUA = -llua

# automatically link to LuaJIT if it exists
#exists(/usr/lib/x86_64-linux-gnu/libluajit-5.1.a):LIBLUA = -L/usr/lib/x86_64-linux-gnu/ -lluajit-5.1

TEMPLATE = app
TARGET = mudlet
RESOURCES = mudlet_alpha.qrc

# try -O1 —fsanitize=address for AddressSanitizer w/ clang
# use -DDEBUG_TELNET to show telnet commands

# Specify default location for Lua files, in OS specific LUA_DEFAULT_DIR value
# below, if this is not done then a hardcoded default of a ./mudlet-lua/lua
# from the executable's location will be used.  Mudlet will now moan and ask
# the user to find them if the files (and specifically the <10KByte
# "LuaGlobal.lua" one) is not accessable (read access only required) during
# startup.  The precise directory is remembered once found (and stored in the
# Mudlet configuration file as "systemLuaFilePath") but if the installer places
# the files in the place documented here the user will not be bothered by this.
#
# (Geyser files should be in a "geyser" subdirectory of this)

unix: {
# Distribution packagers would be using PREFIX = /usr but this is accepted
# destination place for local builds for software for all users:
    isEmpty( PREFIX ) PREFIX = /usr/local
    isEmpty( DATAROOTDIR ) DATAROOTDIR = $${PREFIX}/share
    isEmpty( DATADIR ) DATADIR = $${DATAROOTDIR}/mudlet
# According to Linux FHS /usr/local/games is an alternative location for leasure time BINARIES 8-):
    isEmpty( BINDIR ) BINDIR = $${PREFIX}/bin
# Again according to FHS /usr/local/share/games is the corresponding place for locally built games documentation:
    isEmpty( DOCDIR ) DOCDIR = $${DATAROOTDIR}/doc/mudlet
    LIBS += -lpcre \
        $$LIBLUA \
        -lhunspell \
        -L/usr/local/lib/ \
        -lyajl \
        -lGLU \
        -lzip \
        -lz
    INCLUDEPATH += /usr/include/lua5.1
    LUA_DEFAULT_DIR = $${DATADIR}/lua
    SOURCES += lua-yajl2-linux.c
    ! contains( $${PWD}, $${OUT_PWD} ) {
       message("Copying scripts to build directory...")
       system("cp $${PWD}/rewriteMakefile.* $${OUT_PWD}")
    }
} else:win32: {
    LIBS += -L"C:\\mudlet5_package" \
        -L"C:\\mingw32\\lib" \
        -llua51 \
        -lpcre \
        -lhunspell \
        -llibzip \
        -lzlib \
        -llibzip \
        -L"C:\\mudlet5_package\\yajl-master\\yajl-2.0.5\\lib" \
        -lyajl
    INCLUDEPATH += "c:\\mudlet_package_MINGW\\Lua_src\\include" \
        "C:\\mingw32\\include" \
        "c:\\mudlet_package_MINGW\\zlib-1.2.5" \
        "C:\\mudlet5_package\\boost_1_54_0" \
        "c:\\mudlet_package_MINGW\\pcre-8.0-lib\\include" \
        "C:\\mudlet5_package\\yajl-master\\yajl-2.0.5\\include" \
        "C:\\mudlet5_package\\libzip-0.11.1\\lib" \
        "C:\\mudlet_package_MINGW\\hunspell-1.3.1\\src"
# Leave this undefined so mudlet::readSettings() preprocessing will fall back to
# hard-coded executable's /mudlet-lua/lua/ subdirectory
#    LUA_DEFAULT_DIR = $$clean_path($$system(echo %ProgramFiles%)/lua)
    SOURCES += lua_yajl.c
}

unix {
#   the "target" install set is handled automagically, just not very well...
    target.path = $${BINDIR}
    message("$${TARGET} will be installed to "$${target.path}"...")
#     DOCS.path = $${DOCS_DIR}
#     message("Documentation will be installed to "$${DOCS.path}"...")
    !isEmpty( LUA_DEFAULT_DIR ) {
# if a directory has been set for the lua files move the detail into the
# installation details for the unix case:
        LUA.path = $${LUA_DEFAULT_DIR}
        LUA_GEYSER.path = $${LUA.path}/geyser
# and define a preprocessor symbol LUA_DEFAULT_PATH with the value:
        DEFINES += LUA_DEFAULT_PATH=\\\"$${LUA_DEFAULT_DIR}\\\"
# and say what will happen:
        message("Lua files will be installed to "$${LUA.path}"...")
        message("Geyser lua files will be installed to "$${LUA_GEYSER.path}"...")
    }
}

INCLUDEPATH += irc/include

SOURCES += TConsole.cpp \
    ctelnet.cpp \
    main.cpp \
    Host.cpp \
    HostManager.cpp \
    HostPool.cpp \
    dlgConnectionProfiles.cpp \
    dlgTriggerEditor.cpp \
    TTrigger.cpp \
    TriggerUnit.cpp \
    TLuaInterpreter.cpp \
    dlgTriggersMainArea.cpp \
    dlgOptionsAreaTriggers.cpp \
    dlgOptionsAreaTimers.cpp \
    dlgOptionsAreaScripts.cpp \
    TCommandLine.cpp \
    TTreeWidget.cpp \
    TTreeWidgetItem.cpp \
    TTimer.cpp \
    TScript.cpp \
    TAlias.cpp \
    dlgTimersMainArea.cpp \
    dlgSystemMessageArea.cpp \
    dlgSourceEditorArea.cpp \
    TimerUnit.cpp \
    ScriptUnit.cpp \
    AliasUnit.cpp \
    dlgScriptsMainArea.cpp \
    dlgAliasMainArea.cpp \
    dlgOptionsAreaAlias.cpp \
    dlgSearchArea.cpp \
    TAction.cpp \
    ActionUnit.cpp \
    dlgActionMainArea.cpp \
    dlgOptionsAreaAction.cpp \
    EAction.cpp \
    dlgAboutDialog.cpp \
    TDebug.cpp \
    dlgKeysMainArea.cpp \
    TKey.cpp \
    KeyUnit.cpp \
    dlgProfilePreferences.cpp \
    TTextEdit.cpp \
    XMLexport.cpp \
    XMLimport.cpp \
    FontManager.cpp \
    TFlipButton.cpp \
    TToolBar.cpp \
    TLabel.cpp \
    TEasyButtonBar.cpp \
    TForkedProcess.cpp \
    dlgColorTrigger.cpp \
    dlgTriggerPatternEdit.cpp \
    TSplitter.cpp \
    TSplitterHandle.cpp \
    mudlet.cpp \
    dlgNotepad.cpp \
    THighlighter.cpp \
    dlgComposer.cpp \
    TArea.cpp \
    glwidget.cpp \
    dlgMapper.cpp \
    TRoom.cpp \
    TMap.cpp \
    TBuffer.cpp \
    T2DMap.cpp \
    dlgRoomExits.cpp \
    dlgPackageExporter.cpp \
    exitstreewidget.cpp \
    TRoomDB.cpp \
    TVar.cpp \
    LuaInterface.cpp \
    VarUnit.cpp \
    dlgVarsMainArea.cpp \
    irc/src/ircbuffer.cpp \
    irc/src/irc.cpp \
    irc/src/ircsession.cpp \
    irc/src/ircutil.cpp \
    dlgIRC.cpp


HEADERS += mudlet.h \
    TTimer.h \
    EAction.h \
    TConsole.h \
    ctelnet.h \
    Host.h \
    TMap.h \
    TAStar.h \
    HostManager.h \
    HostPool.h \
    dlgConnectionProfiles.h \
    dlgTriggerEditor.h \
    TTrigger.h \
    TLuaInterpreter.h \
    dlgTriggers_main_area.h \
    dlgOptionsAreaTriggers.h \
    dlgTriggerPatternEdit.h \
    TCommandLine.h \
    TTreeWidget.h \
    TTreeWidgetItem.h \
    TScript.h \
    TAlias.h \
    dlgTimersMainArea.h \
    dlgSourceEditorArea.h \
    dlgSystemMessageArea.h \
    TimerUnit.h \
    ScriptUnit.h \
    AliasUnit.h \
    dlgScriptsMainArea.h \
    dlgAliasMainArea.h \
    dlgOptionsAreaAlias.h \
    dlgOptionsAreaScripts.h \
    dlgOptionsAreaTimers.h \
    dlgSearchArea.h \
    TAction.h \
    ActionUnit.h \
    dlgActionMainArea.h \
    dlgOptionsAreaAction.h \
    dlgAboutDialog.h \
    TMatchState.h \
    TEvent.h \
    TDebug.h \
    dlgKeysMainArea.h \
    TKey.h \
    KeyUnit.h \
    dlgProfilePreferences.h \
    TTextEdit.h \
    TFlipButton.h \
    TToolBar.h \
    TBuffer.h \
    TriggerUnit.h \
    TLabel.h \
    TEasyButtonBar.h \
    TForkedProcess.h \
    dlgColorTrigger.h \
    TSplitter.h \
    TSplitterHandle.h \
    dlgNotepad.h \
    THighlighter.h \
    dlgComposer.h \
    TRoom.h \
    TArea.h \
    TMap.h \
    glwidget.h \
    dlgMapper.h \
    Tree.h \
    dlgIRC.h \
    T2DMap.h \
    dlgRoomExits.h \
    dlgPackageExporter.h \
    exitstreewidget.h \
    TRoomDB.h \
    TVar.h \
    LuaInterface.h \
    VarUnit.h \
    dlgVarsMainArea.h \
    irc/include/ircbuffer.h \
    irc/include/irc.h \
    irc/include/ircsession.h \
    irc/include/ircutil.h


FORMS += ui/connection_profiles.ui \
    ui/main_window.ui \
    ui/trigger_editor.ui \
    ui/options_area_triggers.ui \
    ui/options_area_timers.ui \
    ui/options_area_aliases.ui \
    ui/options_area_scripts.ui \
    ui/triggers_main_area.ui \
    ui/scripts_main_area.ui \
    ui/aliases_main_area.ui \
    ui/system_message_area.ui \
    ui/source_editor_area.ui \
    ui/extended_search_area.ui \
    ui/actions_main_area.ui \
    ui/options_area_actions.ui \
    ui/timers_main_area.ui \
    ui/about_dialog.ui \
    ui/keybindings_main_area.ui \
    ui/color_trigger.ui \
    ui/notes_editor.ui \
    ui/trigger_pattern_edit.ui \
    ui/composer.ui \
    ui/mapper.ui \
    ui/profile_preferences.ui \
    ui/irc.ui \
    ui/mapper_room_color.ui \
    ui/room_exits.ui \
    ui/lacking_mapper_script.ui \
    ui/package_manager.ui \
    ui/module_manager.ui \
    ui/package_manager_unpack.ui \
    ui/dlgPackageExporter.ui \
    ui/custom_lines.ui \
    ui/vars_main_area.ui

# Documentation files:
# DOCS.files =

# Main lua files:
LUA.files = \
    $${PWD}/mudlet-lua/lua/LuaGlobal.lua \
    $${PWD}/mudlet-lua/lua/StringUtils.lua \
    $${PWD}/mudlet-lua/lua/TableUtils.lua \
    $${PWD}/mudlet-lua/lua/DebugTools.lua \
    $${PWD}/mudlet-lua/lua/DB.lua \
    $${PWD}/mudlet-lua/lua/GUIUtils.lua \
    $${PWD}/mudlet-lua/lua/Other.lua \
    $${PWD}/mudlet-lua/lua/GMCP.lua
LUA.depends = mudlet

# Geyser lua files:
LUA_GEYSER.files = \
    $${PWD}/mudlet-lua/lua/geyser/Geyser.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserGeyser.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserUtil.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserColor.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserSetConstraints.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserContainer.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserWindow.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserLabel.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserGauge.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserMiniConsole.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserMapper.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserReposition.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserHBox.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserVBox.lua \
    $${PWD}/mudlet-lua/lua/geyser/GeyserTests.lua
LUA_GEYSER.depends = mudlet

# Pull the docs and lua files into the project so they show up in the Qt Creator project files list
OTHER_FILES += \
#     ${DOCS.files} \
    ${LUA.files} \
    ${LUA_GEYSER.files}

# Unix Makefile installer:
# lua file installation, needs install, sudo, and a setting in /etc/sudo.conf
# or via enviromental variable SUDO_ASKPASS to something like ssh-askpass
# to provide a graphic password requestor needed to install software
unix {
# say what we want to get installed by "make install" (executed by 'deployment' step):
    INSTALLS += target LUA LUA_GEYSER
}
# Other OS's have other installation routines - perhap they could be duplicated here?

