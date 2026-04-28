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
    widgetsizetool.cpp \
    tablewidget.cpp \
    tableview.cpp
HEADERS +=        \
    globals.h     \
    dialog.h      \
    widgetsizetool.h \
    tablewidget.h \
    tableview.h

INCLUDEPATH += $$PWD/

win32:CONFIG(release, debug|release):    DESTDIR = $$PROJECT_ROOT_DIRECTORY/release
else:win32:CONFIG(debug, debug|release): DESTDIR = $$PROJECT_ROOT_DIRECTORY/debug

win32:CONFIG(release, debug|release):    DESTDIR2 = $$PROJECT_ROOT_DIRECTORY/release/config
else:win32:CONFIG(debug, debug|release): DESTDIR2 = $$PROJECT_ROOT_DIRECTORY/debug/config

win32:CONFIG(release, debug|release):    OTHER_FILES = globals.json
else:win32:CONFIG(debug, debug|release): OTHER_FILES = globals.json

win32:CONFIG(release, debug|release):    INSTALLDIR = $$PROJECT_ROOT_DIRECTORY/release
else:win32:CONFIG(debug, debug|release): INSTALLDIR = $$PROJECT_ROOT_DIRECTORY/debug

QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/globals.json)) $$system_path($$quote($$DESTDIR2) $$escape_expand(\\n\\t))
