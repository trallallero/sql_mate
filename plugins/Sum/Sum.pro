include(../config.pri)
include(../../config.pri)

TEMPLATE        = lib
CONFIG         += plugin sql
QT             += core widgets
INCLUDEPATH    += . $$PROJECT_SOURCE_DIRECTORY $$PROJECT_ROOT_DIRECTORY/globals
HEADERS         = sum.h
SOURCES         = sum.cpp
FORMS          += sum.ui
TARGET          = $$qtLibraryTarget(sumplugin)
OTHER_FILES     = sumMenuIcon.png

win32:CONFIG(release, debug|release):    LIBS += -L$$PROJECT_ROOT_DIRECTORY/release/ -lglobalslib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PROJECT_ROOT_DIRECTORY/debug/ -lglobalslib

win32:CONFIG(release, debug|release):    DESTDIR = ../../release/plugins/sum
else:win32:CONFIG(debug, debug|release): DESTDIR = ../../debug/plugins/sum

QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/sumMenuIcon.png)) $$system_path($$quote($$DESTDIR) $$escape_expand(\\n\\t))
