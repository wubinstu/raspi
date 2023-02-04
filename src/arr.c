#include "arr.h"


bool isNotes (const char msg[])
{
    if (msg[0] == '#')
        return true;
    else return false;
}

bool isEmptyL (const char msg[])
{
    if (msg[0] == '\n' || msg[0] == '\r')
        return true;
    else return false;
}

bool isContainC (char *msg, char c)
{
    char *master = msg;
    bool flag = false;
    while (*master != '\0')
        if (*master++ == c)
            flag = true;
    return flag;
}

bool notASCII (char *msg)
{
    char *master = msg;
    bool flag = false;
    while (*master != '\0')
        if ((*master < 0) || (*master++ > 127))
            flag = true;
    return flag;
}

void rmNextL (char *msg)
{
    char *master = msg;
    while (*master != '\0')
    {
        if (*master == '\n')
        {
            *master = '\0';
            break;
        }
        master++;
    }
}

void rmCharacter (char *msg, char c)
{
    char *master = msg;
    char *i, *j;
    while (*master != '\0')
    {
        i = master;
        j = i + 1;
        if (*i == c)
        {
            while (*i != '\0')
            {
                *i = *j;
                i++;
                j++;
            }
        }
        if ((*master) != c)
            master++;
    }
}

void subString (char *msg, char c, char *s1, char *s2)
{
    char *master = msg;
    char *i = s1;
    char *j = s2;
    while (*master != c)
    {
        *i = *master;
        i++;
        master++;
    }
    *i = '\0';
    master++;
    while (*master != '\0')
    {
        *j = *master;
        j++;
        master++;
    }
    if (*(j - 1) == '\n')
        *(j - 1) = '\0';
    else *j = '\0';
}

void upperConversion (char *msg)
{
    char *master = msg;

    while (*master != '\0')
    {
        if ((*master >= 97) && (*master <= 122))
            *master -= 32;
        master++;
    }
}

void lowerConversion (char *msg)
{
    char *master = msg;

    while (*master != '\0')
    {
        if ((*master >= 65) && (*master <= 90))
            *master += 32;
        master++;
    }
}