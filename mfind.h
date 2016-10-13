#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>


#include "list.h"

typedef struct pathRecord {
    char* path;
    bool searched;
} pathRecord;

typedef struct resultRecord{
    char* path;
} resultRecord;

void * search(void * arg);

void pathRecordFree(void *recordToFree);

void resultRecordFree(void *recordToFree);