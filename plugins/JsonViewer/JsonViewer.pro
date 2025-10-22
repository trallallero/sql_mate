include(../config.pri)
include(../../config.pri)

TEMPLATE        = lib
CONFIG         += plugin sql
QT             += core widgets
INCLUDEPATH    += . $$PROJECT_SOURCE_DIRECTORY $$PROJECT_ROOT_DIRECTORY/globals
HEADERS         =   \
    jsonmodel.h \
    jsonviewer.h
SOURCES         =  \
    jsonmodel.cpp \
    jsonviewer.cpp

FORMS          += jsonviewer.ui
TARGET          = $$qtLibraryTarget(jsonviewerplugin)
OTHER_FILES     = jsonviewerMenuIcon.png

win32:CONFIG(release, debug|release):    LIBS += -L$$PROJECT_ROOT_DIRECTORY/release/ -lglobalslib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PROJECT_ROOT_DIRECTORY/debug/ -lglobalslib

win32:CONFIG(release, debug|release):    DESTDIR = ../../release/plugins/JsonViewer
else:win32:CONFIG(debug, debug|release): DESTDIR = ../../debug/plugins/JsonViewer

QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/jsonviewerMenuIcon.png)) $$system_path($$quote($$DESTDIR) $$escape_expand(\\n\\t))
