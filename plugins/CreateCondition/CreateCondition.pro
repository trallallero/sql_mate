include(../config.pri)
include(../../config.pri)

TEMPLATE        = lib
CONFIG         += plugin sql
QT             += core widgets
INCLUDEPATH    += . $$PROJECT_SOURCE_DIRECTORY $$PROJECT_ROOT_DIRECTORY/globals
HEADERS         = createcondition.h
SOURCES         = createcondition.cpp
FORMS          += createcondition.ui
TARGET          = $$qtLibraryTarget(createconditionplugin)
OTHER_FILES     = createConditionMenuIcon.png

win32:CONFIG(release, debug|release):    LIBS += -L$$PROJECT_ROOT_DIRECTORY/release/ -lglobalslib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PROJECT_ROOT_DIRECTORY/debug/ -lglobalslib

win32:CONFIG(release, debug|release):    DESTDIR = ../../release/plugins/createcondition
else:win32:CONFIG(debug, debug|release): DESTDIR = ../../debug/plugins/createcondition

QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/createConditionMenuIcon.png)) $$system_path($$quote($$DESTDIR) $$escape_expand(\\n\\t))
