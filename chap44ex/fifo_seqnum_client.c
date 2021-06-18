#include "../include/fifo_seqnum.h"

static char clientFifo[CLIENT_FIFO_NAME_LEN];

static void
removeFifo(void)
{
    unlink(clientFifo);
}

int
main(int argc, char *argv[])
{
    int serverFd, clientFd;
    struct request req;
    struct response resp;
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usageErr("client [seq-len]");
    }

    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE,
             (long) getpid());
    
    if (mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST) {
        errExit("mkfifo");
    }

    if (atexit(removeFifo) != 0) {
        errExit("atexit");
    }

    req.pid = getpid();
    req.seqLen = (argc > 1) ? getInt(argv[1], GN_GT_0, "seq-len") : 1;

    serverFd = open(SERVER_FIFO, O_WRONLY);
    if (serverFd == -1) {
        errExit("open serverfifo");
    }

    if (write(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
        fatal("could not write to server");
    }

    clientFd = open(clientFifo, O_RDONLY);
    if (clientFd == -1) {
        errExit("open client fifo");
    }

    if (read(clientFd, &resp, sizeof(struct response)) != sizeof(struct response)) {
        fatal("could not read response");
    }

    printf("%d\n", resp.seqNum);
    exit(EXIT_SUCCESS);

}