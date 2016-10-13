/*
 * list.c
 *
 * Lorenz Gerber
 *
 * Laboration 1, Course 'System-level programming', HT16
 *
 * code file consists an implementation of a double linked list using
 * generic/void data fields.
 *
 */

#include "list.h"


//create empty list, constructor
list *listEmpty(void) {
    list *newList=malloc(sizeof(struct list));
    newList->head=malloc(sizeof(element)); //huvudet för listan
    newList->head->next=newList->head;
    newList->head->previous=newList->head;
    newList->freeFunc=NULL;
    return newList;
}


// set mem handler
void listSetMemHandler(list *l, memFreeFunc *f){
    l->freeFunc=f;
}


// return first position
listPosition listFirst(list *l){
    return l->head->next;
}

// return last position
listPosition listLast(list *l){
    return l->head->previous;
}

// get next position
listPosition listNext(listPosition p) {
    return p->next;
}

// get previous position
listPosition listPrevious(listPosition p){
    return p->previous;
}

// is list empty
bool listIsEmpty(list *l) {
    return (l->head->next == l->head);
}

// insert list element
listPosition listInsert(listPosition p,data d) {
    listPosition newPosition=malloc(sizeof(element));
    newPosition->data=d;
    newPosition->next=p->next;
    (p->next)->previous=newPosition;
    newPosition->previous=p;
    p->next=newPosition;
    return newPosition;

}

// inspect list element
data* listInspect(listPosition p) {
  return p->data;
}

// remove list element
listPosition listRemove(list *l, listPosition p) {
    listPosition beforeRemove=p->previous;
    p->previous->next=p->next;
    p->next->previous=p->previous;
    if(l->freeFunc!=NULL)
        l->freeFunc(p->data);
    free(p);
    return beforeRemove;
}

// free memory
void listFree(list *l){
    listPosition current = listLast(l);
    while (listIsEmpty(l) != true){
        current = listRemove(l, current);
    }
    free(l->head);
    free(l);
}



// check if end of list
bool listIsEnd(list *l, listPosition p){
  if ( p->next == l->head) {
    return true;
  } else {
    return false;
  }
}
