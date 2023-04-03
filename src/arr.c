#include "arr.h"

/// 将文件读取出来的某一行的第一个字符和#比对以判断是否为注释
bool isNotes (const char msg[])
{
    return (msg[0] == '#');
}

bool isEmptyL (const char msg[])
{
    return (msg[0] == '\n' || (msg[0] == '\r' && msg[1] == '\n'));
}

bool isContainC (char * msg, char c)
{
    if (msg == NULL)return false;
    char * master = msg;
    bool flag = false;
    while (* master != '\0')
        if (* master++ == c)
            flag = true;
    return flag;
}

bool notASCII (char * msg)
{
    if (msg == NULL) return true;
    char * master = msg;
    bool flag = false;
    while (* master != '\0')
        if ((* master < 0) || (* master++ > 127))
            flag = true;
    return flag;
}

void rmNextL (char * msg)
{
    if (msg == NULL)return;
    char * master = msg;
    while (* master != '\0')
    {
        if (* master == '\n')
        {
            * master = '\0';
            break;
        }
        master++;
    }
}

void rmCharacter (char * msg, char c)
{
    if (msg == NULL)return;
    char * master = msg;
    char * i, * j;
    while (* master != '\0')
    {
        i = master;
        j = i + 1;
        if (* i == c)
        {
            while (* i != '\0')
            {
                * i = * j;
                i++;
                j++;
            }
        }
        if ((* master) != c)
            master++;
    }
}

void subString (char * msg, char c, char * s1, char * s2)
{
    if (msg == NULL || s1 == NULL || s2 == NULL)
        return;
    char * master = msg;
    char * i = s1;
    char * j = s2;
    while (* master != c)
    {
        * i = * master;
        i++;
        master++;
    }
    * i = '\0';
    master++;
    while (* master != '\0')
    {
        * j = * master;
        j++;
        master++;
    }
    if (* (j - 1) == '\n')
        * (j - 1) = '\0';
    else * j = '\0';
}

void upperConversion (char * msg)
{
    if (msg == NULL)
        return;
    char * master = msg;
    while (* master != '\0')
    {
        if ((* master >= 97) && (* master <= 122))
            * master -= 32;
        master++;
    }
}

void lowerConversion (char * msg)
{
    if (msg == NULL)
        return;

    char * master = msg;
    while (* master != '\0')
    {
        if ((* master >= 65) && (* master <= 90))
            * master += 32;
        master++;
    }
}

int lengthToNextSpace (char * msg)
{
    if (msg == NULL)
        return 0;

    char * master = msg;
    int len = 0;
    while (* master != ' ')
        len++, master++;

    return len;
}

int lengthToNextLine (char * msg)
{
    if (msg == NULL)
        return 0;

    char * master = msg;
    int len = 0;
    while (* master != '\n' && (* master != '\r' || * (master + 1) != '\n'))
        len++, master++;

    return len;
}

int moveToNextLine (char ** msg)
{
    if (msg == NULL || * msg == NULL)
        return 0;

    int len = 0;
    while (** msg != '\n' && (** msg != '\r' || ** (msg + 1) != '\n'))
        len++, (* msg)++;

    (* msg)++;
    if (** msg == '\n')
        (* msg)++;
    return len;
}

int igStrCmp (char * s1, char * s2, int len)
{
    if (s1 == NULL || s2 == NULL || len <= 0)
        return 0;

    char c1, c2;
    while (* s1 != '\0' && * s2 != '\0' && len != 0)
    {
        if (* s1 >= 65 && * s1 <= 90)
            c1 = * s1 + 32;
        else c1 = * s1;

        if (* s2 >= 65 && * s2 <= 90)
            c2 = * s2 + 32;
        else c2 = * s2;

        if (c1 != c2)
            return c1 - c2;

        s1++, s2++;
        len--;
    }
    return 0;
}