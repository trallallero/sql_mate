#ifndef QUERIES_H
#define QUERIES_H

#include <QString>
#include <QStringList>

QString tenantsQueryYin =
"SELECT nspname FROM pg_namespace pg WHERE pg.nspname NOT LIKE 'pg_%' AND pg.nspname <> 'information_schema'";

QString tenantsQuerySolidus =
"SELECT nspname FROM pg_namespace pg WHERE pg.nspname LIKE '%_db' AND pg.nspname NOT LIKE 'nome%'";

// YIN QUERIES
QString yin_select_user_id =
"SELECT id FROM {tenant}.customer WHERE email = '{email}'";

QString yin_get_email_by_name =
"SELECT \
    email AS \"Email\" \
FROM \
    {tenant}.customer \
WHERE \
    first_name = '{name}' \
AND last_name  = '{surname}' \
";

QString yin_get_values_by_email =
"SELECT \
    first_name AS \"Name\", \
    last_name  AS \"Surname\" \
FROM \
    {tenant}.customer \
WHERE \
    email = '{email}' \
";

QString yin_select_orders =
"SELECT order_number FROM {tenant}.order WHERE customer_id = {yin_user_id}";

QString yin_delete_customer_seller =
"DELETE FROM {tenant}.customer_seller WHERE customer_id = {yin_user_id}";

QString yin_delete_customer =
"DELETE FROM {tenant}.customer WHERE id = {yin_user_id}";

QString yin_delete_orders =
"DELETE FROM {tenant}.order WHERE customer_id = {yin_user_id}";

// SOLIDUS QUERIES
QString solidus_select_user_id =
"SELECT id FROM {tenant}.spree_users WHERE email = '{email}'";

QString solidus_get_values_by_email =
"SELECT \
    firstname AS \"Name\", \
    lastname  AS \"Surname\" \
FROM \
    {tenant}.spree_users \
WHERE \
    email = '{email}' \
";

QString solidus_select_orders =
"SELECT \
    so.\"number\", \
    sli.policy_number \
FROM \
          {tenant}.spree_orders     so \
LEFT JOIN {tenant}.spree_line_items sli ON sli.order_id = so.id \
WHERE \
    so.user_id = {solidus_user_id} \
";

QString solidus_delete_yolo_user_acceptances =
"DELETE FROM {tenant}.yolo_user_acceptances WHERE user_id = {solidus_user_id}";

QString solidus_delete_yolo_insurance_infos =
"DELETE FROM {tenant}.yolo_insurance_infos WHERE spree_line_item_id IN ( \
    SELECT id FROM {tenant}.spree_line_items WHERE order_id IN ( \
        SELECT id FROM {tenant}.spree_orders WHERE user_id = {solidus_user_id} \
    ) \
) \
";

QString solidus_delete_spree_line_items =
"DELETE FROM {tenant}.spree_line_items WHERE order_id IN ( \
    SELECT id FROM {tenant}.spree_orders WHERE user_id = {solidus_user_id} \
) \
";

QString solidus_delete_spree_orders =
"DELETE FROM {tenant}.spree_orders WHERE user_id = {solidus_user_id}";

QString solidus_delete_spree_addresses_ship =
"DELETE FROM {tenant}.spree_addresses WHERE id = ( \
    SELECT ship_address_id FROM {tenant}.spree_users WHERE id = {solidus_user_id} \
) \
";

QString solidus_delete_spree_addresses_bill =
"DELETE FROM {tenant}.spree_addresses WHERE id = ( \
    SELECT bill_address_id FROM {tenant}.spree_users WHERE id = {solidus_user_id} \
) \
";

QString solidus_delete_spree_users =
"DELETE FROM {tenant}.spree_users WHERE id = {solidus_user_id}";

#endif // QUERIES_H
