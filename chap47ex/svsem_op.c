#include <sys/types.h>
#include <sys/sem.h>
#include <ctype.h>
#include "../include/tlpi_hdr.h"
#include "../include/curr_time.h"

#define MAX_SEMOPS 1000

static void
usagetError(const char *progName)
{
    fprintf(stderr, "Usage");
}

static int
parseOps(char *arg, struct sembuf sops[])
{
    char *comma, *sign, *remaining, *flags;
    int numOps;

    for (numOps = 0, remaining = arg;;numOps++) {
        if (numOps >= MAX_SEMOPS) {
            cmdLineErr("too many ops");
        }

        if (*remaining == '\0') {
            fatal("comma");
        }

        if (!isdigit((unsigned char) *remaining)) {
            cmdLineErr("expected digit");
        }

        sops[numOps].sem_num = strtol(remaining, &sign, 10);

        if (*sign == '\0' || strchr("+-=", *sign) == NULL) {
            cmdLineErr("sign");
        }

        sops[numOps].sem_op = strtol(sign + 1, &flags, 10);
        if (*sign == '-') {
            sops[numOps].sem_op = - sops[numOps].sem_op;
        } else if (*sign == '=') {
            if (sops[numOps].sem_op != 0) {
                cmdLineErr("arg");
            }
        }

        sops[numOps].sem_flg = 0;
        for (;; flags++) {
            if (*flags == 'n') {
                sops[numOps].sem_flg |= IPC_NOWAIT;
            } else if (*flags == 'u') {
                sops[numOps].sem_flg |= SEM_UNDO;
            } else{
                break;
            }
        }

        if (*flags != ',' && *flags != '\0') {
            cmdLineErr("bad character");
        }

        comma = strchr(remaining, ',');
        if (comma == NULL) {
            break;
        } else {
            remaining = comma + 1;
        }
    }
    return numOps + 1;
}

int main(int argc, char *argv[])
{
    struct sembuf sops[MAX_SEMOPS];
    int ind, nsops;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("usage");
    }

    for (ind = 2; argv[ind] != NULL; ind++) {
        nsops = parseOps(argv[ind], sops);
        printf("%5ld, %s: about to semop() [%s]\n", (long) getpid(), currTime("%T"), argv[ind]);

        if (semop(getInt(argv[1], 0, "semid"), sops, nsops) == -1) {
            errExit("semop (PID=%ld)", (long) getpid());
        }

        printf("%5ld, %s: semop() completed [%s]\n", (long) getpid(), currTime("%T"), argv[ind]);
    }

    exit(EXIT_SUCCESS);
}
