#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include "../include/semun.h"
#include "../include/tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct semid_ds ds;
    union semun arg, dummy;
    int semid, j;
    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("usage");
    }

    semid = getInt(argv[1], 0, "semid");

    arg.buf = &ds;
    if (semctl(semid, 0, IPC_STAT, arg) == -1) {
        errExit("semctl");
    }
    printf("semaphore changed: %s", ctime(&ds.sem_ctime));
    printf("last semop(): %s", ctime(&ds.sem_otime));

    arg.array = calloc(ds.sem_nsems, sizeof(arg.array[0]));
    if (arg.array == NULL) {
        errExit("calloc");
    }
    if (semctl(semid, 0, GETALL, arg) == -1){
        errExit("semctl-GETALL");
    }

    printf("sem # value SEMPID SEMNCNT SEMZCNT\n");

    for (j=0; j < ds.sem_nsems; j++) {
        printf("%3d %5d %5d %5d %5d\n", j, arg.array[j], 
                semctl(semid, j, GETPID, dummy), 
                semctl(semid, j, GETNCNT, dummy),
                semctl(semid, j, GETZCNT, dummy));
    }
    exit(EXIT_SUCCESS);
}