#define _GUN_SOURCE
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
    int msqid, flags, type;
    ssize_t msgLen;
    size_t maxBytes;
    struct mbuf msg;
    int opt;

    flags = 0;
    type = 0;
    while((opt = getopt(argc, argv, "ent:x")) != -1) {
        switch (opt) {
        case 'e': flags |= MSG_NOERROR; break;
        case 'n': flags |= IPC_NOWAIT; break;
        case 't': type = atoi(optarg); break;
#ifdef MSG_EXCEPT
        case 'x': flags |= MSG_EXCEPT;
#endif
        default: usageErr("usage");
        }
    }

    if (argc < optind + 1 || argc > optind + 2) {
        usageErr("number");
    }

    msqid = getInt(argv[optind], 0, "msqid");
    maxBytes = (argc > optind + 1) ? getInt(argv[optind], 0, "max-bytes") : MAX_MTEXT;

    msgLen = msgrcv(msqid, &msg, maxBytes, type, flags);
    if (msgLen == -1) {
        errExit("msgrcv");
    }

    printf("Received: type=%ld; length=%ld", msg.mtype, (long) msgLen);
    if (msgLen > 0) {
        printf("; body=%s", msg.mtext);
    }
    printf("\n");
    
    exit(EXIT_SUCCESS);
}
