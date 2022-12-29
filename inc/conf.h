/**
 * Create By Einc On March 23, 2022
 * This file provides functions for simple processing of configuration files,
 * Recommended call order:
 * 1.checkconf
 * 2.readconf
 * 3.checkread */

#ifndef __CONF_H_
#define __CONF_H_

#include "head.h"
#include "mylink.h"


 /**
  * Regardless of whether the file exists or not and whether the content of the file exists,
  * overwrite creates a default configuration file
  * 覆盖性新建一个默认的配置文件 */
extern void defaultconf (const char* file_path);

/**
 * Read the configuration file and write the content to the linked list,
 * and then return the address of the first node of the linked list.
 * If the reading fails, return null
 * 读取配置文件并将配置项以链表方式返回 */
extern LNode readconf (const char* file_path);

/**
 * Check whether the file exists.
 * If it does exits, return true;
 * Else if it does not exist, create a default configuration file and return false
 * 检擦配置文件是否存在,不存在会自动创建默认的配置文件 */
extern bool checkconf (const char* file_path);

/**
 * Enter the address of the first node of the linked list returned by the readconf function.
 * If it is an empty linked list, return false.
 * If it is not empty, check the data field of each node one by one.
 * If there are non ASCII characters, return false.
 * Therefore, return true if and only if the linked list is not empty and there are no non ASCII characters in all node data fields.
 * Note: for the whole program, this is only a preliminary judgment, and whether the data fully meet the requirements is not judged
 * 检擦链表里面的配置项的合法性
 * 当链表为空或任意数据域含有非ASCII字符时返回假
 * 这只是初步检查,不能检擦键值对中的值的逻辑合法性,所以这是个用处不大的函数 */
extern bool checkread (LNode L);

#endif