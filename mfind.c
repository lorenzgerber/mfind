/**
 * mfind.c
 *
 * Lorenz Gerber
 *
 * Laboration 4, 5DV088, HT16
 *
 * main code file for program mfind that has a similar functionality
 * as for example the find program. The programs takes as command line
 * argument two flags, -t for type of file to search for (f: file,
 * l: link, d: directory) and -p for number of threads to use. Further, at
 * least one start search path has to be indicated as well as the file to
 * search for.
 */
#define _POSIX_C_SOURCE 200809L
#include "mfind.h"


pthread_mutex_t mtx_pathList;
pthread_mutex_t mtx_resultList;
semaphore n;
list *pathList;
list *resultList;
char type = 0;
char *toMatch;


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
    int nrthr = 0;
    int matchSet = false;

    /*
     * Parse commandline options
     */
    while ((opt = getopt(argc, argv, "t:p:")) != -1) {
        switch (opt) {
            case 't':
                type = *optarg;
                matchSet = true;
                break;
            case 'p':
                nrthr = atoi(optarg);
                if(nrthr == 0){
                    fprintf(stderr, "Usage: mfind [-t type] [-p nrthr] start1 [start2 ...] name\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Usage: mfind [-t type] [-p nrthr] start1 [start2 ...] name\n");
                exit(EXIT_FAILURE);
        }
    }

    if(type != 100 && type != 102 && type != 108 && matchSet){
        fprintf(stderr, "Usage: mfind [-t type] [-p nrthr] start1 [start2 ...] name\n");
        exit(EXIT_FAILURE);
    } else {
        type = 0;
    }

    if (argc - 1 == optind){
        fprintf(stderr, "Usage: mfind [-t type] [-p nrthr] start1 [start2 ...] name\n");
        exit(EXIT_FAILURE);
    }

    if (optind >= argc) {
        fprintf(stderr, "Usage: mfind [-t type] [-p nrthr] start1 [start2 ...] name\n");
        exit(EXIT_FAILURE);
    }

    /*
     * Initialize data- and synchronization
     * structures
     */
    pathList = listEmpty();
    listSetMemHandler(pathList, pathRecordFree);
    resultList = listEmpty();
    listSetMemHandler(resultList, resultRecordFree);

    if ((n = initsem(1001)) == -1) {
        perror("Can't create semaphore");
        exit(1);
    }
    pthread_mutex_init(&mtx_pathList, NULL);
    pthread_mutex_init(&mtx_resultList, NULL);



    /*
     * Check validity of provided path's
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

        /*
         * Add path's to pathList
         * and signal semaphore
         */
        if(insertPath(argv[iii])<0){
            fprintf(stderr, "insertPath error\n");
        }

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
    pthread_t threads[nrthr];
    threads[0] = pthread_self();

    for(int iii = 1; iii < nrthr;iii++){
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
    for(int iii = 1; iii < nrthr;iii++){
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

    /*
     * Clean-up
     */
    killsem(n);
    listFree(pathList);
    listFree(resultList);
    free(toMatch);
    exit(EXIT_SUCCESS);
}

/**
 * threadMain
 *
 * @param dummy
 * @return
 */
void *threadMain(void *dummy){
    int callToOpenDir = 0;
    do{
        int check = semwait(n);
        if (check == -1) {
            perror("Error waiting for semaphore");
        } else if(check != EAGAIN){
            callToOpenDir += readDirectory();
        }

    } while (semctl(n, 0, GETVAL) > 0 );
    printf("Thread: %ld Reads: %d\n",(int long)pthread_self(), callToOpenDir);
    return NULL;
}

/**
 * readDirectory
 *
 * @return
 */
int readDirectory(void){
    int callToOpendir = 0;

    /*
     *  get the path to work on
     */
    listPosition currentPosition;
    getWorkPath(&currentPosition);

    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    char * currentPath = ((pathRecord*)listInspect(currentPosition))->path;

    /*
     * Check if we have access permission
     */
    if(access(currentPath, R_OK|X_OK) < 0){
        perror("access");
        return callToOpendir;
    }

    /*
     * Check if we can read the directory
     */
    if((dp = opendir(currentPath))==NULL){
        fprintf(stderr, "can't read directory\n");
        return callToOpendir;
    } else {
        callToOpendir++;
    }

    /*
     * Loop through directory
     */
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
         * Check directory match
         */
        if (S_ISDIR(statbuf.st_mode) == 1){
            if(matched && (type == 100 || type == 0)){
                if(insertResult(fullpath)<0){
                    fprintf(stderr,"insert Result error\n");
                }
            }

            /*
             * Add Directory to path list
             */
            if(insertPath(fullpath)<0){
                fprintf(stderr,"insert Result error\n");
            }

            if (semsignal(n) == -1) {
                perror("Error signalling the semaphore");
                exit(1);
            }
        /*
         * Check file match
         */
        } else if(S_ISREG(statbuf.st_mode) ==1){
            if(matched && (type == 102 || type == 0)){

                if(insertResult(fullpath)<0){
                    fprintf(stderr,"insert Result error\n");
                }
            }
        /*
         * Check link match
         */
        } else if(S_ISLNK(statbuf.st_mode) == 1){
            if(matched && (type == 108 || type == 0)){
                if(insertResult(fullpath)<0){
                    fprintf(stderr,"insert Result error\n");
                }
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

    return callToOpendir;
}

/**
 * getWorkPath
 *
 * @param workPath
 * @return
 */
int getWorkPath(listPosition *workPath){
    pthread_mutex_lock(&mtx_pathList);
    listPosition currentPosition = listFirst(pathList);
    bool foundOrEnd = false;
    // find path to work on
    while(foundOrEnd == false){
        if(((pathRecord*)listInspect(currentPosition))->searched == false){
            pathRecord* workRecord = (pathRecord*)listInspect(currentPosition);
            workRecord->searched = true;

            foundOrEnd = true;
        } else if (listIsEnd(pathList,currentPosition)==true ){
            printf("när händer det?\n");
            foundOrEnd = true;
        } else {
            currentPosition = listNext(currentPosition);
        }
    }
    pthread_mutex_unlock(&mtx_pathList);

    *workPath = currentPosition;
    return 0;

}

/**
 *  insert Result
 *
 * @param fullpath
 * @return
 */
int insertResult(char* fullpath){
    char* insertionPath = strdup(fullpath);
    resultRecord *insertionRecord = malloc(sizeof(resultRecord) * 1);
    insertionRecord->path = insertionPath;
    pthread_mutex_lock(&mtx_resultList);
    listInsert(listLast(resultList), (data)insertionRecord);
    pthread_mutex_unlock(&mtx_resultList);
    return 0;
}

/**
 * Insert Path
 *
 * @param fullpath
 * @return
 */
int insertPath(char* fullpath){
    char* insertionPath = strdup(fullpath);
    pathRecord *insertionRecord = malloc(sizeof(pathRecord) * 1);
    insertionRecord->path = insertionPath;
    insertionRecord->searched = false;
    pthread_mutex_lock(&mtx_pathList);
    listInsert(listLast(pathList), (data)insertionRecord);
    pthread_mutex_unlock(&mtx_pathList);
    return 0;
}

