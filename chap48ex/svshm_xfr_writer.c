#include "../include/semun.h"
#include "../include/svshm_xfr.h"

int
main(int argc, char *argv[])
{
    int semid, shmid, bytes, xfrs;
    struct shmseg  *shmp;
    union semun dummy;

    semid = semget(SEM_KEY, 2, IPC_CREAT | OBJ_PERMS);
    if (semid == -1) {
        errExit("semget");
    }

    if (initSemAvailable(semid, WRITE_SEM) == -1) {
        errExit("init sem");
    }
    if (initSemInUse(semid, READ_SEM) == -1) {
        errExit("init sem");
    }

    shmid = shmget(SHM_KEY, sizeof(struct shmseg), IPC_CREAT | OBJ_PERMS);
    if (shmid == -1) {
        errExit("shmget");
    }

    shmp = shmat(shmid, NULL, 0);
    if (shmp == (void *) -1) {
        errExit("shmat");
    }

    for (xfrs = 0, bytes = 0;;xfrs++, bytes += shmp->cnt) {
        if (reserveSem(semid, WRITE_SEM) == -1) {
            errExit("reserveSem");
        }

        shmp->cnt = read(STDIN_FILENO, shmp->buf, BUF_SIZE);
        if (shmp->cnt == -1) {
            errExit("read");
        }

        if (releaseSem(semid, READ_SEM) == -1) {
            errExit("releaseSem");
        }

        if (shmp->cnt == 0) {
            break;
        }
    }

    if (releaseSem(semid, WRITE_SEM) == -1) {
        errExit("reserveSem");
    }

    if (semctl(semid, 0, IPC_RMID, dummy) == -1) {
        errExit("rm");
    }

    if (shmdt(shmp) == -1) {
        errExit("shmdt");
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        errExit("shmctl");
    }

    fprintf(stderr, "sent %d bytes (%d xfrs)\n", bytes, xfrs);
    exit(EXIT_SUCCESS);
}