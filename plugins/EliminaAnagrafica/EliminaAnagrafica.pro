include(../config.pri)
include(../../config.pri)

TEMPLATE        = lib
CONFIG         += plugin sql
QT             += core widgets sql
INCLUDEPATH    += . $$PROJECT_SOURCE_DIRECTORY $$PROJECT_ROOT_DIRECTORY/globals
HEADERS         = \
    eliminaanagrafica.h \
    queries.h \
    deletequeriesviewer.h
SOURCES         = \
    eliminaanagrafica.cpp \
    deletequeriesviewer.cpp
FORMS          += \
    eliminaanagrafica.ui \
    deletequeriesviewer.ui
TARGET          = $$qtLibraryTarget(eliminaanagraficaplugin)
OTHER_FILES     = eliminaanagraficaMenuIcon.png
RESOURCES      += resources.qrc

win32:CONFIG(release, debug|release):    LIBS += -L$$PROJECT_ROOT_DIRECTORY/release/ -lglobalslib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PROJECT_ROOT_DIRECTORY/debug/ -lglobalslib

win32:CONFIG(release, debug|release):    DESTDIR = $$PROJECT_ROOT_DIRECTORY/release/plugins/EliminaAnagrafica
else:win32:CONFIG(debug, debug|release): DESTDIR = $$PROJECT_ROOT_DIRECTORY/debug/plugins/EliminaAnagrafica

QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/eliminaanagraficaMenuIcon.png)) $$system_path($$quote($$DESTDIR) $$escape_expand(\\n\\t))
