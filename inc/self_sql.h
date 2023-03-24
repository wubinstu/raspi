//
// Created by Einc on 2023/03/21.
//

#ifndef __SELF_SQL_H_
#define __SELF_SQL_H_


extern bool mysql_lib (bool true_for_init_false_for_end);

extern MYSQL * mysql_server_connection (MYSQL * sql);

extern int mysql_insert_values (MYSQL * sql, const char * table_name,
                                const char * fields, const char * values);


extern int mysql_select_query (MYSQL * sql, const char * table_name,
                               const char * fields_list, const char * join_method,
                               const char * join_table, const char * join_keys,
                               const char * where_condition, const char * order_condition,
                               const char * group_condition);

#endif //__SELF_SQL_H_
