/**
 * mfind.c
 *
 * Lorenz Gerber
 *
 * Laboration 4, 5DV088, HT16
 *
 * header file for mfind program
 *
 */
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
#include <limits.h>
#include <sys/stat.h>
#include <pthread.h>
#include <limits.h>
#include <sys/stat.h>
#include "semops.h"

#include "list.h"

#ifdef PATH_MAX
static long pathmax = PATH_MAX;
#else
static long pathmax = 4096;
#endif



typedef struct pathRecord {
    char* path;
    bool searched;
} pathRecord;

typedef struct resultRecord{
    char* path;
} resultRecord;

void *threadMain(void *dummy);

void pathRecordFree(void *recordToFree);

void resultRecordFree(void *recordToFree);

int readDirectory(void);

int getWorkPath(listPosition *workPath);

int insertResult(char* fullpath);

int insertPath(char* fullpath);
