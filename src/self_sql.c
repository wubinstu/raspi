//
// Created by Einc on 2023/03/21.
//
#include "head.h"
#include "global.h"
#include "self_sql.h"
#include "stdbool.h"
#include "mysql/mysql.h"
#include "log.h"

bool mysql_lib (bool true_for_init_false_for_end)
{
    if (true_for_init_false_for_end)
    {
        if (mysql_library_init (0, NULL, NULL))
            return false;
        else return true;
    } else
        mysql_library_end ();
    return true;
}

MYSQL * mysql_server_connection (MYSQL * sql)
{
    return mysql_real_connect
            (sql, config_server.sqlHost,
             config_server.sqlUser,
             config_server.sqlPass,
             config_server.sqlName,
             config_server.sqlPort,
             NULL, CLIENT_SSL);
}

int mysql_insert_values (MYSQL * sql, const char * table_name,
                         const char * fields, const char * values)
{
    char * cmd = calloc (1, 1024);
    if (cmd == NULL)
        return -1;
    strcpy (cmd, "insert into ");
    strcat (cmd, table_name);
    strcat (cmd, " ");
    if (fields != NULL)
        strcat (cmd, fields), strcat (cmd, " ");
    strcat (cmd, "values ");
    strcat (cmd, values);
    int rtn = mysql_query (sql, cmd);
    free (cmd);
    return rtn;
}

int mysql_select_query (MYSQL * sql, const char * table_name,
                        const char * fields_list, const char * join_method,
                        const char * join_table, const char * join_keys,
                        const char * where_condition, const char * order_condition,
                        const char * group_condition)
{
    char * cmd = calloc (1, 1024);

    strcpy (cmd, "select ");
    strcat (cmd, fields_list);
    strcat (cmd, " from ");
    strcat (cmd, table_name);
    if (join_method != NULL)
    {
        strcat (cmd, " ");
        strcat (cmd, join_method);
        strcat (cmd, " ");
        strcat (cmd, join_table);
        strcat (cmd, " on ");
        strcat (cmd, join_keys);
    }
    if (where_condition != NULL)
    {
        strcat (cmd, " where ");
        strcat (cmd, where_condition);
    }
    if (order_condition != NULL)
    {
        strcat (cmd, " order by ");
        strcat (cmd, order_condition);
    }
    if (group_condition != NULL)
    {
        strcat (cmd, " group by ");
        strcat (cmd, group_condition);
    }
    int rtn;
    rtn = mysql_query (sql, cmd);
    free (cmd);
    return rtn;
}