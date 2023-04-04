/**
 * Create By Einc On March 23, 2022
 * This file provides common string processing functions */


#ifndef __ARR_H_
#define __ARR_H_

#include "head.h"

/**
 * It is used to determine the comment in the configuration file.
 * When the first character of the incoming string is '#', it returns true */
extern bool isNotes (const char msg[]);

/**
 * It is used to determine the empty line in the configuration file.
 * When the first character of the incoming string is \\n, it returns true */
extern bool isEmptyL (const char msg[]);

/**
 * Scan the string and return true if it contains the specified character */
extern bool isContainC (char * msg, char c);

/**
 * Scan the string and return true if it contains the character which is NOT ASCII */
extern bool notASCII (char * msg);

/**
 * It is used to determine the comment in the configuration file.
 * Replace the first \\n in the incoming string with \\0 */
extern void rmNextL (char * msg);

/**
 * Scans the string and deletes the specified character */
extern void rmCharacter (char * msg, char c);

/**
 * Cut the string 'msg' from the specified character 'c' (which must exist),
 * and assign the two substrings to 's1' and 's2' respectively */
extern void subString (char * msg, char c, char * s1, char * s2);

/**
 * Converts all lowercase letters in a string to uppercase letters*/
extern void upperConversion (char * msg);

/**
 * Converts all uppercase letters in a string to lowercase letters*/
extern void lowerConversion (char * msg);

/**
 * count number to next space: ' ' */
extern int lengthToNextSpace (char * msg);

/**
 * Count number to next line: '\\n', "\\r\\n" */
extern int lengthToNextLine (char * msg);

/** Move pointer strings to next line */
extern int moveToNextLine (char ** msg);

/** String comparison that ignore case */
extern int igStrCmp (char * s1, char * s2, int len);

#endif