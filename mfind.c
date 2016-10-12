#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

int main(int argc, char *argv[]) {

    /*
     * Initialize variables
     */
    int type, tfnd;
    int nrthr, pfnd;

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
                type = atoi(optarg);
                tfnd = 1;
                break;
            case 'p':
                nrthr = atoi(optarg);
                pfnd = 1;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }



    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    if(access(argv[optind], F_OK)<0){
        perror(access);
        exit(EXIT_FAILURE);
    }

    printf("name argument = %s\n", argv[optind]);



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




    printf("this is the shit\n");
    exit(EXIT_SUCCESS);
}




int search(){

    return 0;
}