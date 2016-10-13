#define _POSIX_C_SOURCE 200809L
#include "mfind.h"
#include <pthread.h>


void pathRecordFree(void *recordToFree){
    pathRecord *record = (pathRecord*)recordToFree;
    free(record->path);
    free(record);
}
void resultRecordFree(void *recordToFree){
    pathRecord *record = (pathRecord*)recordToFree;
    free(record->path);
    free(record);
}


int main(int argc, char *argv[]) {

    /*
     * Initialize variables
     */
    char type;
    int tfnd;
    int nrthr, pfnd;
    list *pathList = listEmpty();
    listSetMemHandler(pathList, pathRecordFree);
    list *resultList = listEmpty();
    listSetMemHandler(resultList, resultRecordFree);

    tfnd = 0;
    pfnd = 0;
    int opt;


    /*
     * Parse commandline options
     *
     * mfind [-t type] [-p nrthr] start1 [start ...] name
     *
     */
    while ((opt = getopt(argc, argv, "t:p:")) != -1) {
        switch (opt) {
            case 't':
                type = *optarg;
                tfnd = 1;
                break;
            case 'p':

                nrthr = atoi(optarg);
                pfnd = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }


    if(tfnd){
        printf("type set to %c\n", type);
    }

    if(pfnd){
        printf("number of threads %d\n", nrthr);
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    for (int iii = argc - 2 ; iii >= optind; iii--){
        if(access(argv[iii], F_OK)<0){
            perror("acces");
            exit(EXIT_FAILURE);
        }

        char* insertionPath = strdup(argv[iii]);
        pathRecord *insertionRecord = malloc(sizeof(pathRecord) * 1);
        insertionRecord->path = insertionPath;
        insertionRecord->searched = false;
        listInsert(listLast(pathList), (data)insertionRecord);

    }

    pthread_t threads[nrthr];
    threads[0] = pthread_self();

    /*
     * Start all threads
     *
     * Control access to shared data structures
     *
     * Check for end of work
     *
     * print out results
     *
     */
    for(int iii = 1; iii < nrthr;iii++){
        pthread_create(&threads[iii], NULL, search, NULL);

    }







    printf("this is the shit\n");
    exit(EXIT_SUCCESS);
}



/*
 * Thread main function
 *
 * 1. Put all start folders in 'folder list' to be traversed
 *
 * 2. working function
 * - take a map from list of folders
 * - read entries, write relevant into result list
 * - if entry is a map, put into folder list
 * - increase counter of folders searched
 * - start again
 *
 */
void* search(void * arg){
    printf("in the thread");

    return ((void *)0);
}
