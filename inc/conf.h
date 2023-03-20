/**
 * Create By Einc On March 23, 2022
 * This file provides functions for simple processing of configuration files,
 * Recommended call order:
 * 1.checkConf
 * 2.readConf
 * 3.checkRead */

#ifndef __CONF_H_
#define __CONF_H_

#include "linklist.h"

/**
 * Read the configuration file and write the content to the linked list,
 * and then return the address of the first node of the linked list.
 * If the reading fails, return null */
extern LNode readConf (const char * file_path);


/**
 * Regardless of whether the file exists or not and whether the content of the file exists,
 * overwrite creates a default configuration file */
extern void defaultConfServ (const char * file_path);

extern void defaultConfClnt (const char * file_path);

/**
 * Check whether the file exists.
 * If it does exits, return true;
 * Else if it does not exist, create a default configuration file and return false */
extern bool checkConf (const char * file_path);

/**
 * Enter the address of the first node of the linked list returned by the readConf function.
 * If it is an empty linked list, return false.
 * If it is not empty, check the data field of each node one by one.
 * If there are non ASCII characters, return false.
 * Therefore, return true if and only if the linked list is not empty and there are no non ASCII characters in all node data fields.
 * Note: for the whole program, this is only a preliminary judgment, and whether the data fully meet the requirements is not judged */
extern bool checkRead (LNode L);

#endif