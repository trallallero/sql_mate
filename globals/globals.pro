include(../config.pri)

DEFINES += GLOBALSLIB_LIBRARY

QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = globalslib
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = lib
SOURCES +=          \
    globals.cpp     \
    dialog.cpp      \
    tablewidget.cpp \
    widgetsizetool.cpp
HEADERS +=        \
    globals.h     \
    dialog.h      \
    tablewidget.h \
    widgetsizetool.h

INCLUDEPATH += $$PWD/

win32:CONFIG(release, debug|release):    DESTDIR = $$PROJECT_ROOT_DIRECTORY/release
else:win32:CONFIG(debug, debug|release): DESTDIR = $$PROJECT_ROOT_DIRECTORY/debug

win32:CONFIG(release, debug|release):    DESTDIR_CONFIG = $$PROJECT_ROOT_DIRECTORY/release/config
else:win32:CONFIG(debug, debug|release): DESTDIR_CONFIG = $$PROJECT_ROOT_DIRECTORY/debug/config

SOURCEDIR_TEMPLATE = $$PROJECT_ROOT_DIRECTORY/profiles/template

win32:CONFIG(release, debug|release):    DESTDIR_PROFILES = $$PROJECT_ROOT_DIRECTORY/release/profiles
else:win32:CONFIG(debug, debug|release): DESTDIR_PROFILES = $$PROJECT_ROOT_DIRECTORY/debug/profiles

win32:CONFIG(release, debug|release):    DESTDIR_TEMPLATE = $$PROJECT_ROOT_DIRECTORY/release/profiles/template
else:win32:CONFIG(debug, debug|release): DESTDIR_TEMPLATE = $$PROJECT_ROOT_DIRECTORY/debug/profiles/template

win32:CONFIG(release, debug|release):    OTHER_FILES = globals.json
else:win32:CONFIG(debug, debug|release): OTHER_FILES = globals.json

win32:CONFIG(release, debug|release):    INSTALLDIR = $$PROJECT_ROOT_DIRECTORY/release
else:win32:CONFIG(debug, debug|release): INSTALLDIR = $$PROJECT_ROOT_DIRECTORY/debug

!exists($$system_path($$quote($${DESTDIR_CONFIG}))) {
    QMAKE_PRE_LINK += $$QMAKE_MKDIR $$system_path($$quote($$DESTDIR_CONFIG))
}

!exists($$system_path($$quote($${DESTDIR_PROFILES}))) {
    QMAKE_POST_LINK +=    $$QMAKE_MKDIR $$system_path($$quote($$DESTDIR_PROFILES))
    QMAKE_POST_LINK += && $$QMAKE_MKDIR $$system_path($$quote($$DESTDIR_TEMPLATE))

    QMAKE_POST_LINK += && $$QMAKE_COPY_DIR $$system_path($$quote($$SOURCEDIR_TEMPLATE)) $$system_path($$quote($$DESTDIR_TEMPLATE))
    QMAKE_POST_LINK += && $$QMAKE_COPY $$system_path($$quote($$PWD/globals.json)) $$system_path($$quote($$DESTDIR_CONFIG) $$escape_expand(\\n\\t))
} else {
    QMAKE_POST_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/globals.json)) $$system_path($$quote($$DESTDIR_CONFIG) $$escape_expand(\\n\\t))
}
