include(../config.pri)
include(../../config.pri)

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets sql
INCLUDEPATH    += . $$PROJECT_SOURCE_DIRECTORY $$PROJECT_ROOT_DIRECTORY/globals
HEADERS         = braintree.h
SOURCES         = braintree.cpp
FORMS          += braintree.ui
TARGET          = $$qtLibraryTarget(braintreeplugin)
OTHER_FILES     = brainTreeMenuIcon.png \
                  braintree_fetcher.py \
                  config.json
RESOURCES      += resources.qrc

win32:CONFIG(release, debug|release):    LIBS += -L$$PROJECT_ROOT_DIRECTORY/release/ -lglobalslib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PROJECT_ROOT_DIRECTORY/debug/ -lglobalslib

win32:CONFIG(release, debug|release):    DESTDIR = ../../release/plugins/braintree
else:win32:CONFIG(debug, debug|release): DESTDIR = ../../debug/plugins/braintree

QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/brainTreeMenuIcon.png)) $$system_path($$quote($$DESTDIR) $$escape_expand(\\n\\t))
QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/braintree_fetcher.py))  $$system_path($$quote($$DESTDIR) $$escape_expand(\\n\\t))
QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PWD/config.json))           $$system_path($$quote($$DESTDIR) $$escape_expand(\\n\\t))

