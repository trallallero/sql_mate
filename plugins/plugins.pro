include(../config.pri)

TEMPLATE    = subdirs
SUBDIRS     =  \
    BrainTree \
    AddToCondition \
    CreateCondition \
    SelectFromTable \
    Sum \
    EliminaAnagrafica \
    JsonViewer

CONFIG += lrelease
CONFIG += embed_translations
