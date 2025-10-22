include(../config.pri)
include(../../config.pri)

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets sql
INCLUDEPATH    += . $$PROJECT_SOURCE_DIRECTORY $$PROJECT_ROOT_DIRECTORY/globals
HEADERS         = selectfromtable.h
SOURCES         = selectfromtable.cpp
FORMS          += selectfromtable.ui
TARGET          = $$qtLibraryTarget(selectfromtable)
OTHER_FILES     = selectFromTableMenuIcon.png
RESOURCES      += resources.qrc

win32:CONFIG(release, debug|release):    LIBS += -L$$PROJECT_ROOT_DIRECTORY/release/ -lglobalslib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PROJECT_ROOT_DIRECTORY/debug/ -lglobalslib

win32:CONFIG(release, debug|release):    DESTDIR = ../../release/plugins/selectfromtable
else:win32:CONFIG(debug, debug|release): DESTDIR = ../../debug/plugins/selectfromtable

QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/selectFromTableMenuIcon.png)) $$system_path($$quote($$DESTDIR) $$escape_expand(\\n\\t))
