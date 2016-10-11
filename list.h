/*
 * list.h
 *
 * Lorenz Gerber
 *
 * Laboration 1, Course 'System-level programming', HT16
 *
 * header file for implementation of a double linked list using
 * generic/void data fields.
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


typedef void *data;

typedef void memFreeFunc(data);

// element for a double linked list
typedef struct element {
    struct element *previous;
    data data;
    struct element *next;
} element;

typedef element * listPosition;

// head for a double linked list
typedef struct list {
    element *head;
    memFreeFunc *freeFunc;
} list;



//create empty list, constructor
list *listEmpty(void);


// set mem handler
void listSetMemHandler(list *l, memFreeFunc *f);


// return first position
listPosition listFirst(list *l);

// reutrn last position
listPosition listLast(list *l);

// get next position
listPosition listNext(list *l, listPosition p);

// get previous position
listPosition listPrevious(list *l, listPosition p);

// is list empty
bool listIsEmpty(list *l);

// insert list element
listPosition listInsert(list *l,listPosition p,data d);

// inspect list element
data* listInspect(list *l, listPosition p);

// remove list element
listPosition listRemove(list *l, listPosition p);

// free memory
void listFree(list *l);

// check if end of list
bool listIsEnd(list *l, listPosition p);