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
 * Read the configuration file and write the content to the linked list,
 * and then return the address of the first node of the linked list.
 * If the reading fails, return null */
extern LNode readconf(const char * file_path);

/**
 * Regardless of whether the file exists or not and whether the content of the file exists,
 * overwrite creates a default configuration file */
extern void defaultconf(const char * file_path);

/**
 * Check whether the file exists.
 * If it does exits, return true;
 * Else if it does not exist, create a default configuration file and return false */
extern bool checkconf(const char * file_path);

/**
 * Enter the address of the first node of the linked list returned by the readconf function.
 * If it is an empty linked list, return false.
 * If it is not empty, check the data field of each node one by one.
 * If there are non ASCII characters, return false.
 * Therefore, return true if and only if the linked list is not empty and there are no non ASCII characters in all node data fields.
 * Note: for the whole program, this is only a preliminary judgment, and whether the data fully meet the requirements is not judged */
extern bool checkread(LNode L);

#endif