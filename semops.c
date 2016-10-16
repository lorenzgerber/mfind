#include    "semops.h"

semaphore initsem(int key)
{
    return semget(key, 1, 0600 | IPC_CREAT);
}

int semwait(semaphore semid)
{
    struct sembuf   swait = {0, -1, 0};

    return semop(semid, &swait, 1);
}

int semsignal(semaphore semid)
{
    struct sembuf   ssignal = {0, 1, 0};

    return semop(semid, &ssignal, 1);
}

int killsem(semaphore semid)
{
    return semctl(semid, 1, IPC_RMID);
}
