#include <sys/types.h>
#include <sys/msg.h>
#include "../include/tlpi_hdr.h"

#define MAX_MTEXT 1024

struct mbuf {
    long mtype;
    char mtext[MAX_MTEXT];
};

int main(int argc, char *argv[])
{
    int msqid, flags, msgLen;
    struct mbuf msg;
    int opt;

    flags = 0;
    while ((opt = getopt(argc, argv, "n")) != -1) {
        /* code */
        if (opt == 'n') {
            flags |= IPC_NOWAIT;
        } else {
            usageErr("usage");
        }
    }

    if (argc < optind + 2 || argc > optind + 3) {
        usageErr("arg number");
    }

    msqid = getInt(argv[optind], 0, "msqid");
    msg.mtype = getInt(argv[optind + 1], 0, "msg-type");

    if (argc > optind + 2) {
        msgLen = strlen(argv[optind + 2]) + 1;
        if (msgLen > MAX_MTEXT) {
            cmdLineErr("msg too long");
        }

        memcpy(msg.mtext, argv[optind + 2], msgLen);
    } else {
        msgLen = 0;
    }

    if (msgsnd(msqid, &msg, msgLen, flags) == -1) {
        errExit("msgsnd");
    }

    exit(EXIT_SUCCESS);
}
