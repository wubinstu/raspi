/**
 * Create By Einc On March 23, 2022
 * This file provides common string processing functions */


#ifndef __ARR_H_
#define __ARR_H_

#include "head.h"

 /**
  * It is used to determine the comment in the configuration file.
  * When the first character of the incoming string is '#', it returns true
  * 检测字符串第一个字符串是否是以'#'开始的,若是返回真
  * 这个函数用作在读取配置文件时判定注释行使用的 */
extern bool isNotes (const char msg[]);

/**
 * It is used to determine the empty line in the configuration file.
 * When the first character of the incoming string is \\n, it returns true
 * 判断一个字符串是否是以换行符,回车符开始的,若是返回真
 * 这个函数用作在读取配置文件时判定空行时使用的 */
extern bool isEmptyL (const char msg[]);

/**
 * Scan the string and return true if it contains the specified character
 * 判断字符串是否含有指定字符,若含有则返回真
 * 这个函数用作在读取配置文件时识别等于号(赋值号),识别成功则判定为可能是一条有效配置 */
extern bool isContainC (char* msg, char c);

/**
 * Scan the string and return true if it contains the character which is NOT ASCII
 * 判断字符串内是否含有非ASCII字符,如果有则判定为真
 * 这个函数用作读取配置文件时检测其他(非法)字符的 */
extern bool notASCII (char* msg);

/**
 * It is used to determine the comment in the configuration file.
 * Replace the first \\n in the incoming string with \\0
 * 若字符串末尾有换行符,则使用结束标记替代它
 * 这个函数用于读取配置文件时去除每一行结尾的换行符 */
extern void rmNextL (char* msg);

/**
 * Scans the string and deletes the specified character
 * 删除字符串中所有指定字符
 * 这个函数用作读取配置文件时删除多余空格的问题 */
extern void rmCharacter (char* msg, char c);

/**
 * Cut the string 'msg' from the specified character 'c' (which must exist),
 * and assign the two substrings to 's1' and 's2' respectively
 * 将字符串从指定字符出切割并拷贝并按照前后顺序拷贝到s1和s2中去,注意:字符c将直接丢弃,不会复制到任何一个子串中
 * 这个函数用于读取配置文件时拆分赋值语句 */
extern void subString (char* msg, char c, char* s1, char* s2);

/**
 * Converts all lowercase letters in a string to uppercase letters
 * 全部转大写 */
extern void upperConversion (char* msg);

/**
 * Converts all uppercase letters in a string to lowercase letters
 * 全部转小写 */
extern void lowerConversion (char* msg);

#endif