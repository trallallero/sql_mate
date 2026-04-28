include(../config.pri)
include(../../config.pri)

TEMPLATE        = lib
CONFIG         += plugin sql
QT             += core widgets
INCLUDEPATH    += . $$PROJECT_SOURCE_DIRECTORY $$PROJECT_ROOT_DIRECTORY/globals
HEADERS         = addtocondition.h
SOURCES         = addtocondition.cpp
FORMS          += addtocondition.ui
TARGET          = $$qtLibraryTarget(addtoconditionplugin)
OTHER_FILES     = addToConditionMenuIcon.png

win32:CONFIG(release, debug|release):    LIBS += -L$$PROJECT_ROOT_DIRECTORY/release/ -lglobalslib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PROJECT_ROOT_DIRECTORY/debug/ -lglobalslib

win32:CONFIG(release, debug|release):    DESTDIR = ../../release/plugins/addtocondition
else:win32:CONFIG(debug, debug|release): DESTDIR = ../../debug/plugins/addtocondition

QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/addToConditionMenuIcon.png)) $$system_path($$quote($$DESTDIR) $$escape_expand(\\n\\t))
