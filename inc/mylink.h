

#ifndef __LINK_H_
#define __LINK_H_

#include "head.h"
#include "types.h"

/**
 * Define the structure of linked list nodes
 * The use of pointer type data fields will
 * make the linked list more widely applicable
 * */
typedef struct LinkNode
{
    Elem * opt;
    struct LinkNode * next;
}Node,*LNode;

/**
 * Initialize the linked list and
 * allocate available space for the head node */
extern void InitLinkList(LNode * L);

/**
 * Given a certain array and its length, create a linked list,
 * and the created linked list will be returned in the form of
 * pointer through the first parameter */
extern void CreateLinkList(LNode * L,Elem e[],int length);

/**
 * Free the space allocated by all nodes of
 * the whole linked list from the first node */
extern void DestoryLinkList(LNode * L);

/**
 * Return the length of the Given linked list.
 * Note: This refers to the number of effective nodes,
 * excluding the number of remaining nodes of the head node*/
extern int LengthOfLinkList(LNode L);

/**
 * Inserts the given element at the specified location
 * After this operation, the order of the new elements
 * in the valid nodes will be the value of parameter "location".
 * But if the "location" is invalid, then do nothing */
extern void InsertIntoLinkList(LNode * L,int location,Elem e);

/**
 * Add an element at the end of the linked list*/
extern void AddToLinkList(LNode * L,Elem e);

/**
 * Given the linked list and location, return the pointer
 * (Space needs to be allocated in advance)
 * where the element is located through the third parameter
 * if the "location" is invalid, then do nothing */
extern void GetFromLinkList(LNode L,int location,Elem * e);

/**
 * Free  the node and elem by the specified location */
extern void DeleteAtLinkList(LNode * L,int location);

/**
 * Sequential output linked list.
 * Note: it calls the output function suitable
 * for a specific element type, so if you change
 * the data field type, you must update the implementation
 * of the output function synchronously at file "types.c/h" */
extern void DisplayLinkList(LNode L);

/**
 * Write the values in the linked list into the array in order
 * and return the length of the successfully written array.
 * The array space needs to be allocated in advance*/
extern int ListToArry(LNode L,Elem e[]);

#endif