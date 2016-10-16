#define	_XOPEN_SOURCE
#include    <sys/types.h>
#include    <sys/ipc.h>
#include    <sys/sem.h>

typedef int semaphore;

semaphore initsem(int key);
int killsem(semaphore semid);
int semwait(semaphore semid);
int semsignal(semaphore semid);
