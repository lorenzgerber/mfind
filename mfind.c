#define _POSIX_C_SOURCE 200809L
#include "mfind.h"


pthread_mutex_t mtx_pathList;
pthread_mutex_t mtx_resultList;
list *pathList;
list *resultList;
char matchType = 0;
char *toMatch;
semaphore n;

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
    int opt;
    int nrThreads = 0;
    int nrDirectories = 0;

    pathList = listEmpty();
    listSetMemHandler(pathList, pathRecordFree);
    resultList = listEmpty();
    listSetMemHandler(resultList, resultRecordFree);

    pthread_mutex_init(&mtx_pathList, NULL);
    pthread_mutex_init(&mtx_resultList, NULL);

    /*
     * Parse commandline options
     */
    while ((opt = getopt(argc, argv, "t:p:")) != -1) {
        switch (opt) {
            case 't':
                matchType = *optarg;
                break;
            case 'p':
                nrThreads = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if(matchType != 100 && matchType != 102 && matchType != 108){
        matchType = 0;
    }
    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    /*
     * Initialize nrPath Semaphore
     */
    if ((n = initsem(1001)) == -1) {
        perror("Can't create semaphore");
        exit(1);
    }

    /*
     * are the provided path'es valid
     */
    for (int iii = argc - 2 ; iii >= optind; iii--){
        struct stat statbuf;
        if (stat(argv[iii], &statbuf) < 0){
            fprintf(stderr, "lstat: %s\n", argv[iii]);
            perror("");
            exit(EXIT_FAILURE);
        }
        if (S_ISDIR(statbuf.st_mode) == 0){
            fprintf(stderr,"%s is not or does not link to a directory\n", argv[iii]);
            exit(EXIT_FAILURE);
        }

        char* insertionPath = strdup(argv[iii]);
        pathRecord *insertionRecord = malloc(sizeof(pathRecord) * 1);
        insertionRecord->path = insertionPath;
        insertionRecord->searched = false;
        listInsert(listLast(pathList), (data)insertionRecord);
        nrDirectories++;
        if (semsignal(n) == -1) {
            perror("Error signalling the semaphore");
            exit(1);
        }
    }
    /*
     * String to match
     */
    toMatch = strdup(argv[argc-1]);

    /*
     * Start all threads
     */
    pthread_t threads[nrThreads];
    threads[0] = pthread_self();

    for(int iii = 1; iii < nrThreads;iii++){
        if(pthread_create(&threads[iii], NULL, &threadMain, NULL)){
            perror("pthread\n");
            exit(EXIT_FAILURE);
        }
    }

    /*
     * start main function for master thread
     */
    threadMain(NULL);

    /*
     * wait for all threads to finish
     */
    for(int iii = 1; iii < nrThreads;iii++){
        if(pthread_join(threads[iii], NULL)){
            perror("pthread_join");
        }
    }

    /*
     * print the results
     */
    if(listIsEmpty(resultList)){

    } else {
        bool moreResults = true;
        listPosition currentPosition = listFirst(resultList);
        while(moreResults){
            printf("%s\n", ((resultRecord*)listInspect(currentPosition))->path);

            if(listIsEnd(resultList,currentPosition)==true){
                moreResults = false;
            } else {
                currentPosition = listNext(currentPosition);
            }
        }
    }
    killsem(n);
    exit(EXIT_SUCCESS);
}

/*
 * Thread main function
 *
 */
void *threadMain(void *dummy){
    int callToOpenDir = 0;
    do{
        int check = semwait(n);
        if (check == -1) {
            perror("Error waiting for semaphore");
        } else if(check != EAGAIN){
            callToOpenDir += readDir();
        }
    } while (semctl(n, 0, GETVAL) > 0);
    printf("Thread: %ld Reads: %d\n",(int long)pthread_self(), callToOpenDir);
    return NULL;
}

/*
 * adding a path to a the pathList
 */
int readDir(void){
    int callToOpendir = 0;
    pthread_mutex_lock(&mtx_pathList);
    listPosition currentPosition = listFirst(pathList);
    bool foundOrEnd = false;

    // find path to work on
    while(foundOrEnd == false){
        if(((pathRecord*)listInspect(currentPosition))->searched == false){
            pathRecord* workRecord = (pathRecord*)listInspect(currentPosition);
            workRecord->searched = true;

            foundOrEnd = true;
        }
        if(listIsEnd(pathList,currentPosition)==true || foundOrEnd == true){
            foundOrEnd = true;
        } else {
            currentPosition = listNext(currentPosition);
        }
    }
    pthread_mutex_unlock(&mtx_pathList);

    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    char * currentPath = ((pathRecord*)listInspect(currentPosition))->path;
    if(access(currentPath, R_OK|X_OK) < 0){
        perror("access");
    } else {
        if((dp = opendir(currentPath))==NULL){
            fprintf(stderr, "can't read directory\n");
        } else {
            callToOpendir++;
            while ((dirp= readdir(dp)) != NULL){
                if (strcmp(dirp->d_name, ".") == 0  ||
                    strcmp(dirp->d_name, "..") == 0)
                    continue;

                /*
                 * prepare path + name
                 */
                size_t pathLength = strlen(currentPath);
                size_t fileLength = strlen(dirp->d_name);
                int multiplier = 1;
                while((pathLength+fileLength+2) > (pathmax * multiplier)){
                    multiplier ++;
                }
                char* fullpath = calloc(pathmax * multiplier, sizeof(char));
                strcpy(fullpath, currentPath);
                fullpath[pathLength++] = '/';
                strcpy(&fullpath[pathLength], dirp->d_name);

                bool matched = false;

                if(strcmp(dirp->d_name, toMatch)==0){
                    matched = true;
                };

                if(lstat(fullpath, &statbuf)<0){
                    perror("lstat");
                };

                /*
                 * New Directory to search found
                 */
                if (S_ISDIR(statbuf.st_mode) == 1){
                    if(matched && (matchType == 100 || matchType == 0)){

                        char* insertionPath = strdup(fullpath);
                        resultRecord *insertionRecord = malloc(sizeof(resultRecord) * 1);
                        insertionRecord->path = insertionPath;
                        pthread_mutex_lock(&mtx_resultList);
                        listInsert(listLast(resultList), (data)insertionRecord);
                        pthread_mutex_unlock(&mtx_resultList);
                    }

                    // make new pathRecord
                    char* insertionPath = strdup(fullpath);
                    pathRecord *insertionRecord = malloc(sizeof(pathRecord) * 1);
                    insertionRecord->path = insertionPath;
                    insertionRecord->searched = false;
                    pthread_mutex_lock(&mtx_pathList);
                    listInsert(listLast(pathList), (data)insertionRecord);
                    pthread_mutex_unlock(&mtx_pathList);

                    if (semsignal(n) == -1) {
                        perror("Error signalling the semaphore");
                        exit(1);
                    }
                } else if(S_ISREG(statbuf.st_mode) ==1){
                    if(matched && (matchType == 102 || matchType == 0)){

                        char* insertionPath = strdup(fullpath);
                        resultRecord *insertionRecord = malloc(sizeof(resultRecord) * 1);
                        insertionRecord->path = insertionPath;
                        pthread_mutex_lock(&mtx_resultList);
                        listInsert(listLast(resultList), (data)insertionRecord);
                        pthread_mutex_unlock(&mtx_resultList);
                    }
                } else if(S_ISLNK(statbuf.st_mode) == 1){
                    if(matched && (matchType == 108 || matchType == 0)){
                        char* insertionPath = strdup(fullpath);
                        resultRecord *insertionRecord = malloc(sizeof(resultRecord) * 1);
                        insertionRecord->path = insertionPath;
                        pthread_mutex_lock(&mtx_resultList);
                        listInsert(listLast(resultList), (data)insertionRecord);
                        pthread_mutex_unlock(&mtx_resultList);
                    }

                }
                free(fullpath);
            }
            if (closedir(dp) < 0){
                perror("closedir");
            }
            pthread_mutex_lock(&mtx_pathList);
            listRemove(pathList, currentPosition);
            pthread_mutex_unlock(&mtx_pathList);
        }
    }
    return callToOpendir;
}

