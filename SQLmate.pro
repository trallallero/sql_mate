include(config.pri)

TEMPLATE = subdirs
SUBDIRS  = globals \
           main \
           plugins

CONFIG += lrelease
CONFIG += embed_translations

TRANSLATIONS += \
    translations/SqlMate_en.ts
