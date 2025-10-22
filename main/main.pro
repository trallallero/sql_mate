include(../config.pri)

QT += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

UI_DIR = source/UI

SOURCES += \
    source/about.cpp \
    source/conditionfield.cpp \
    source/conditionfieldhandler.cpp \
    source/conditionscontextmenu.cpp \
    source/connections.cpp \
    source/contextmenuwidget.cpp \
    source/filesystemmodel.cpp \
    source/menu.cpp \
    source/newcondition.cpp \
    source/conditions.cpp \
    source/exportresult.cpp \
    source/filterfields.cpp \
    source/main.cpp \
    source/mainwindow.cpp \
    source/popupmenu.cpp \
    source/profiles.cpp \
    source/pushbutton.cpp \
    source/queryviewer.cpp \
    source/sql.cpp \
    source/volatilemessage.cpp \
    source/pluginshandler.cpp \
    source/dragframe.cpp

HEADERS += \
    source/about.h \
    source/conditionfield.h \
    source/conditionfieldhandler.h \
    source/conditionscontextmenu.h \
    source/connections.h \
    source/contextmenuwidget.h \
    source/filesystemmodel.h \
    source/menu.h \
    source/newcondition.h \
    source/conditions.h \
    source/exportresult.h \
    source/filterfields.h \
    source/mainwindow.h \
    source/popupmenu.h \
    source/profiles.h \
    source/pushbutton.h \
    source/queryviewer.h \
    source/sql.h \
    source/volatilemessage.h \
    source/pluginshandler.h \
    source/plugininterface.h \
    source/dragframe.h

FORMS += \
    source/UI/newcondition.ui \
    source/UI/exportresult.ui \
    source/UI/filterfields.ui \
    source/UI/mainwindow.ui \
    source/UI/profiles.ui \
    source/UI/queryviewer.ui \
    source/UI/volatilemessage.ui \
    source/UI/connections.ui \
    source/UI/about.ui

CONFIG += lrelease
CONFIG += embed_translations

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release):    LIBS += -L$$PROJECT_ROOT_DIRECTORY/libs/release/ -lQXlsx
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PROJECT_ROOT_DIRECTORY/libs/debug/ -lQXlsx

win32:CONFIG(release, debug|release):    LIBS += -L$$PROJECT_ROOT_DIRECTORY/release/ -lQXlsx -lglobalslib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PROJECT_ROOT_DIRECTORY/debug/ -lQXlsx -lglobalslib

INCLUDEPATH += $$PWD/libs $$PWD/source $$PROJECT_ROOT_DIRECTORY/globals
DEPENDPATH += $$PWD/libs

win32:CONFIG(release, debug|release):    DESTDIR = ../release
else:win32:CONFIG(debug, debug|release): DESTDIR = ../debug

RESOURCES += resources.qrc

TARGET = SqlMate

win32:RC_ICONS += images/sqlmate.ico

TARGET_CUSTOM_EXT = $${TARGET}.exe
DEPLOY_COMMAND = windeployqt
DEPLOY_OPTIONS = "--no-svg --no-system-d3d-compiler --no-opengl --no-angle --no-opengl-sw"

CONFIG(debug, debug|release) {
    # debug
    DEPLOY_TARGET = $$shell_quote($$shell_path($$PROJECT_ROOT_DIRECTORY/debug/$${TARGET_CUSTOM_EXT}))
    DEPLOY_OPTIONS += "--debug"
} else {
    # release
    DEPLOY_TARGET = $$shell_quote($$shell_path($$PROJECT_ROOT_DIRECTORY/release/$${TARGET_CUSTOM_EXT}))
    DEPLOY_OPTIONS += "--release"
}
QMAKE_POST_LINK += $${DEPLOY_COMMAND} $${DEPLOY_OPTIONS} $${DEPLOY_TARGET}
QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PROJECT_ROOT_DIRECTORY/config/config.json))          $$system_path($$quote($$DESTDIR/config/config.json)         $$escape_expand(\\n\\t))
QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PROJECT_ROOT_DIRECTORY/config/connections.json))     $$system_path($$quote($$DESTDIR/config/connections.json)    $$escape_expand(\\n\\t))
QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PROJECT_ROOT_DIRECTORY/config/filtered_fields.txt))  $$system_path($$quote($$DESTDIR/config/filtered_fields.txt) $$escape_expand(\\n\\t))
QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PROJECT_ROOT_DIRECTORY/config/globals.json))         $$system_path($$quote($$DESTDIR/config/globals.json)        $$escape_expand(\\n\\t))
QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PROJECT_ROOT_DIRECTORY/config/stylesheets.txt))      $$system_path($$quote($$DESTDIR/config/stylesheets.txt)     $$escape_expand(\\n\\t))

QMAKE_PRE_LINK += $$QMAKE_COPY $$system_path($$quote($$PROJECT_ROOT_DIRECTORY/translations/SqlMate_en.qm))  $$system_path($$quote($$DESTDIR/SqlMate_en.qm)              $$escape_expand(\\n\\t))

message($$QMAKE_POST_LINK)
