#ifndef FINDBY_H
#define FINDBY_H

#include <QMap>
#include <QString>

enum FindBy
{
    EMAIL           = 0,
    POL_NUMBER      = 1,
    POL_NUMBER_COMP = 2,
    ORDER_NUMBER    = 3,
    NAME            = 4,
    PRODUCT         = 5,
    TAXON           = 6,
    SUB_CODE        = 7,
    TAXCODE         = 8
};

QMap<FindBy, QString> FindByConditions {
    {
        EMAIL,
        "AND so.email = '{arg_0}'\n"
    },
    {
        POL_NUMBER,
        "AND sli.policy_number = '{arg_0}'\n"
    },
    {
        POL_NUMBER_COMP,
        "AND (so.data->'emission_response'->'envelope'->'body'->'emetti_polizza_response'"
        "->'emetti_polizza_result'->>'n_polizza' = '{arg_0}'\n"
        "OR\n"
        "ro.payload->'emission_response'->'envelope'->'body'->'emetti_polizza_response'"
        "->'emetti_polizza_result'->>'n_polizza' = '{arg_0}')\n"
    },
    {
        ORDER_NUMBER,
        "AND so.number = '{arg_0}'\n"
    },
    {
        NAME,
        "AND su.firstname = '{arg_0}' AND su.lastname = '{arg_1}'\n"
    },
    {
        PRODUCT,
        "AND sp.\"name\" = '{arg_0}'\n",
    },
    {
        TAXON,
        "AND st.\"name\" = '{arg_0}'\n"
    },
    {
        SUB_CODE,
        "AND ys.code = '{arg_0}'\n"
    },
    {
        TAXCODE,
        "AND sa.taxcode = '{arg_0}'\n"
    }
};

#endif // MAINWINDOW_H
