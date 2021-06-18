//#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <netdb.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include "../include/is_seqnum.h"

#define BACKLOG 50

struct mbuf {
    long mtype;
    char mtext[1024];
};

int
main(int argc, char *argv[])
{
    uint32_t seqNum;
    char reqLenStr[INT_LEN];
    char seqNumStr[INT_LEN];
    struct sockaddr_storage claddr;
    int lfd, cfd, optval, reqLen, msgLen;
    int msqid[2];
    int mqid = 0;
    struct mbuf msg[2];
    socklen_t addrlen;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
    char addrStr[ADDRSTRLEN];
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usageErr("%s [init-seq-num]\n", argv[0]);
    }

    seqNum = (argc > 1) ? getInt(argv[1], 0, "init-seq-num") : 0;

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        errExit("signal");
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, PORT_NUM, &hints, &result) != 0) {
        errExit("getaddrinfo");
    }

    optval = 1;

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lfd == -1) {
            continue;
        }

        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            errExit("setsockopt");
        }

        if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }

        close(lfd);
    }

    if (rp == NULL) {
        fatal("could not bind socket to any address");
    }

    if  (listen(lfd, BACKLOG) == -1) {
        errExit("listen");
    }

    freeaddrinfo(result);

    msqid[0] = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR);
    if (msqid[0] == -2) {
        errExit("msgget");
    }
    msqid[1] = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR);
    if (msqid[1] == -2) {
        errExit("msgget");
    }
    setbuf(stdout, NULL);
    for (;;) {
        addrlen = sizeof(struct sockaddr_storage);
        cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen);
        if (cfd == -1) {
            errMsg("accept");
            continue;
        }

        if (getnameinfo((struct sockaddr *) &claddr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
            snprintf(addrStr, ADDRSTRLEN, "(%s, %s)", host, service);
        } else {
            snprintf(addrStr, ADDRSTRLEN, "(?UNKNOWN?)");
        }

        printf("Connection from %s\n", addrStr);

        if (readLine(cfd, reqLenStr, INT_LEN) <= 0) {
            close(cfd);
            continue;
        }
        printf("onnection from %s\n", addrStr);
        
        msg[mqid].mtype = mqid + 5;
        memcpy(msg[mqid].mtext, reqLenStr, strlen(reqLenStr));

        if (msgsnd(msqid[mqid], &msg[mqid], strlen(reqLenStr), 0) == -1) {
            errExit("msgsend");
        }

        printf("send");
        snprintf(seqNumStr, INT_LEN, "%d\n", seqNum);

        printf("not received");
        printf("mqid: %d\n", mqid);
        mqid = abs(mqid-1);
        msgLen = msgrcv(msqid[mqid], &msg[mqid], 1024, mqid + 5, IPC_NOWAIT);
        if (msgLen <= 0){
            // errExit("msgrcv");
            printf("msgLen <= 0\n");
        } else {
            printf("text: %s", msg[mqid].mtext);
            if (write(cfd, msg[mqid].mtext, strlen(msg[mqid].mtext)) != strlen(msg[mqid].mtext)) {
                fprintf(stderr, "Error on write");
            }
        }
        printf("received");

        if (close(cfd) == -1) {
            errMsg("close");
        }
    }
}