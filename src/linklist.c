
#include "global.h"
#include "head.h"
#include "linklist.h"

const int ElemSize = sizeof (KeyValuePair);
const int NodeSize = sizeof (Node);


void InitLinkList (LNode *L)
{
    (*L) = (LNode) calloc (1, NodeSize);
    (*L)->opt = NULL;
    (*L)->next = NULL;
}

void CreateLinkList (LNode *L, KeyValuePair e[], int length)
{
    LNode master, erratic;
    InitLinkList (L);
    master = (*L);
    // The First Node Will NOT Be Used

    for (int i = 0; i < length; i++)
    {
        erratic = (Node *) calloc (1, NodeSize);
        erratic->opt = (KeyValuePair *) calloc (1, ElemSize);
        memcpy ((KeyValuePair *) (erratic->opt), (KeyValuePair *) &e[i], ElemSize);
        erratic->next = NULL;
        master->next = erratic;
        master = erratic;
    }
}

void DestroyLinkList (LNode *L)
{
    LNode master = (*L);
    LNode ahead;

    while (master)
    {
        ahead = master->next;
        free (master);
        master = ahead;
    }
}

int LengthOfLinkList (LNode L)
{
    LNode master = L;
    int length = 0;
    while (master != NULL)
    {
        length++;
        master = master->next;
    }
    return length - 1;
}

void InsertIntoLinkList (LNode *L, int location, KeyValuePair e)
{
    if (location <= 0 || location > LengthOfLinkList (*L))
        return;
    LNode master = *L, erratic;
    int ruler = 1;
    while (ruler < location)
    {
        ruler++;
        master = master->next;
    }
    erratic = calloc (1, NodeSize);
    erratic->opt = calloc (1, ElemSize);
    erratic->next = NULL;
    memcpy ((KeyValuePair *) (erratic->opt), (KeyValuePair *) &e, ElemSize);
    erratic->next = master->next;
    master->next = erratic;
}

void AddToLinkList (LNode *L, KeyValuePair e)
{
    LNode master = (*L), erratic;
    while (master->next != NULL)master = master->next;
    erratic = calloc (1, NodeSize);
    erratic->opt = calloc (1, ElemSize);
    memcpy ((KeyValuePair *) (erratic->opt), (KeyValuePair *) &e, ElemSize);
    erratic->next = NULL;
    master->next = erratic;
}

void GetFromLinkList (LNode L, int location, KeyValuePair *e)
{
    if (location <= 0 || location > LengthOfLinkList (L))
        return;
    LNode master = L;
    int ruler = 0;
    while (ruler < location)
    {
        ruler++;
        master = master->next;
    }
    memcpy ((KeyValuePair *) e, (KeyValuePair *) (master->opt), ElemSize);
}

void DeleteAtLinkList (LNode *L, int location)
{
    if (location <= 0 || location > LengthOfLinkList (*L))
        return;
    LNode master = *L;
    LNode ahead = (*L)->next;
    int ruler = 1;
    while (ruler < location)
    {
        ruler++;
        ahead = ahead->next;
        master = master->next;
    }
    master->next = ahead->next;
    free (ahead);
}

void DisplayLinkList (LNode L)
{
    LNode master = L->next;
    while (master != NULL)
    {
        /* calls the output function suitable for a specific element type */
        catElem (*(KeyValuePair *) master->opt);
        master = master->next;
    }
}

int ListToArry (LNode L, KeyValuePair e[])
{
    int i = 0;
    LNode master = L->next;
    while (master != NULL)
    {
        memcpy (&e[i++], master->opt, ElemSize);
        master = master->next;
    }
    return i;
}