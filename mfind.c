#define _POSIX_C_SOURCE 200809L
#include "mfind.h"


pthread_mutex_t mtx_pathList;
pthread_mutex_t mtx_resultList;
int waitCount = 0;
int nrThreads = 0;
list *pathList;
list *resultList;
char matchType = 0;
char *toMatch;

int totalCount = 0;
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

    int nrDirectories = 0;

    pathList = listEmpty();
    listSetMemHandler(pathList, pathRecordFree);
    resultList = listEmpty();
    listSetMemHandler(resultList, resultRecordFree);

    pthread_mutex_init(&mtx_pathList, NULL);
    pthread_mutex_init(&mtx_resultList, NULL);

    /*
     * Parse commandline options
     *
     * mfind [-t type] [-p nrThreads] start1 [start ...] name
     *
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

    if(matchType == 100 || matchType == 102 || matchType == 108){
        //printf("type set to %c\n", matchType);
    } else {
        printf("unknown type, will match for name\n");
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
    //printf("this is the shit\n");
    //printf("%ld\n", pathmax);

    if(listIsEmpty(resultList)){
        printf("There was no match");
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
    printf("total count %d\n", totalCount );
    killsem(n);
    exit(EXIT_SUCCESS);
}



/*
 * Thread main function
 *
 */
void *threadMain(void *dummy){
    int callToOpenDir = 0;
    int threadsCorrected = nrThreads;
    if(nrThreads==1){
        threadsCorrected++;
    }

    waitCount++;
    int semValue;

    do{


        //printf("%d\n", waitCount);
        int check = semwait(n);

        //printf("%d\n", semValue);

        if (check == -1) {
            perror("Error waiting for semaphore");
            exit(1);
        } else if(check == EAGAIN){
            //printf("no work available\n");
        } else {
            //printf("readDir\n");
            waitCount--;
            callToOpenDir += readDir();

            waitCount++;

        }
        semValue = semctl(n, 0, GETVAL);


    } while (waitCount < threadsCorrected && semValue > 0);

    printf("Thread: %ld Reads: %d\n",(int long)pthread_self(), callToOpenDir);
    totalCount += callToOpenDir;
    return NULL;
}

/*
 * adding a path to a the pathList
 */
int readDir(void){

    /*
    semaphore n;

    if ((n = initsem(1001)) == -1) {
        perror("Can't create semaphore");
        exit(1);
    }*/


    int callToOpendir = 0;
    // lock the pathList
    pthread_mutex_lock(&mtx_pathList);
    //printf("pathlist locked\n");
    listPosition currentPosition = listFirst(pathList);
    bool foundOrEnd = false;

    // find path to work on
    while(foundOrEnd == false){
        if(((pathRecord*)listInspect(currentPosition))->searched == false){
           //printf("we have one to work on\n");
            // mark path as done
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

    // unlock pathlist
    pthread_mutex_unlock(&mtx_pathList);
    //printf("pathlist unlocked\n");

    // loop through directory entries
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
                //printf("%s %d\n", dirp->d_name, (int) strlen(currentPath));

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
                //printf("%s\n", fullpath);

                bool matched = false;
                //checkmatch
                if(strcmp(dirp->d_name, toMatch)==0){
                    //printf("WE GOT A MATCH\n");
                    matched = true;
                };

                // get lstat of current entry
                if(lstat(fullpath, &statbuf)<0){
                    perror("lstat");
                };

                //if directory found
                if (S_ISDIR(statbuf.st_mode) == 1){
                    //printf("%s is a directory\n", fullpath);

                    if(matched && (matchType == 100 || matchType == 0)){

                        char* insertionPath = strdup(fullpath);
                        resultRecord *insertionRecord = malloc(sizeof(resultRecord) * 1);
                        insertionRecord->path = insertionPath;

                        //mutex lock resultList
                        pthread_mutex_lock(&mtx_resultList);
                        //printf("resultlist locked\n");
                        //add to resultList
                        listInsert(listLast(resultList), (data)insertionRecord);

                        //mutex unlock resultList
                        pthread_mutex_unlock(&mtx_resultList);
                        //printf("resultlist unlocked\n");

                    }

                    // make new pathRecord
                    char* insertionPath = strdup(fullpath);
                    pathRecord *insertionRecord = malloc(sizeof(pathRecord) * 1);
                    insertionRecord->path = insertionPath;
                    insertionRecord->searched = false;

                    //mutex lock pathList
                    pthread_mutex_lock(&mtx_pathList);
                    //printf("pathlist locked\n");


                    //add directory to path list
                    listInsert(listLast(pathList), (data)insertionRecord);




                    //mutex unlock pathlist
                    pthread_mutex_unlock(&mtx_pathList);
                    //printf("pathlist unlocked\n");

                    //semaphore post

                    //sem_post(&pathCount);
                    if (semsignal(n) == -1) {
                        perror("Error signalling the semaphore");
                        exit(1);
                    }
                    //printf("signalling\n");


                } else if(S_ISREG(statbuf.st_mode) ==1){
                    //printf("%s is a file\n", fullpath);

                    if(matched && (matchType == 102 || matchType == 0)){

                        char* insertionPath = strdup(fullpath);
                        resultRecord *insertionRecord = malloc(sizeof(resultRecord) * 1);
                        insertionRecord->path = insertionPath;

                        //mutex lock resultList
                        pthread_mutex_lock(&mtx_resultList);
                        //printf("resultlist locked\n");
                        //add to resultList
                        listInsert(listLast(resultList), (data)insertionRecord);

                        //mutex unlock resultList
                        pthread_mutex_unlock(&mtx_resultList);
                        //printf("resultlist unlocked\n");

                    }

                } else if(S_ISLNK(statbuf.st_mode) == 1){
                    //printf("%s is a link\n", fullpath);

                    if(matched && (matchType == 108 || matchType == 0)){

                        char* insertionPath = strdup(fullpath);
                        resultRecord *insertionRecord = malloc(sizeof(resultRecord) * 1);
                        insertionRecord->path = insertionPath;

                        //mutex lock resultList
                        pthread_mutex_lock(&mtx_resultList);
                        //printf("resultlist locked\n");
                        //add to resultList
                        listInsert(listLast(resultList), (data)insertionRecord);

                        //mutex unlock resultList
                        pthread_mutex_unlock(&mtx_resultList);
                        //printf("resultlist unlocked\n");

                    }

                }

                free(fullpath);

            }
            if (closedir(dp) < 0){
                perror("closedir");
            }
            // could remove the directory
            pthread_mutex_lock(&mtx_pathList);
            //printf("pathlist locked\n");
            listRemove(pathList, currentPosition);
            //mutex unlock pathlist
            pthread_mutex_unlock(&mtx_pathList);
            //printf("pathlist unlocked\n");


        }

    }

    return callToOpendir;
}

