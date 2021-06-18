#include <sys/types.h>
#include <sys/sem.h>
#include "../include/semun.h"
#include "../include/tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct semid_ds ds;
    union semun arg;
    int j, semid;
    
    semid = getInt(argv[1], 0, "semid");

    arg.buf = &ds;
    if (semctl(semid, 0, IPC_STAT, arg) == -1) {
        errExit("semctl");
    }

    if (ds.sem_nsems != argc - 2) {
        cmdLineErr("number of sem");
    }

    arg.array = calloc(ds.sem_nsems, sizeof(arg.array[0]));
    if (arg.array == NULL) {
        errExit("calloc");
    }

    for (j = 2; j < argc; j++) {
        arg.array[j-2] = getInt(argv[j], 0, "val");
    }

    if (semctl(semid, 0, SETALL, arg) == -1) {
        errExit("semclt");
    }
    printf("Semaphore values changed (PID=%ld)\n", (long) getpid());

    exit(EXIT_SUCCESS);
}