#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include "../include/tlpi_hdr.h"

#define BUF_SIZE 1000
#define PORT_NUM "50000"

int
main(int argc, char *argv[]) 
{
    int cfd, numread;
    char buf[BUF_SIZE];
    char *msgPrefix;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    if (argc < 3) {
        usageErr("%s addr name\n", argv[0]);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family =  AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    if (getaddrinfo(argv[1], PORT_NUM, &hints, &result) == -1) {
        errExit("getaddrinfo");
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (cfd == -1) {
            continue;
        }

        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            printf("connected server\n");
            break;
        }

        close(cfd);
    }

    if (rp == NULL) {
        fatal("could not connect");
    }

    freeaddrinfo(result);

    msgPrefix = strcat(argv[2], ": ");
    printf("%s\n", msgPrefix);

    switch (fork()) {
    case -1:
        errExit("fork");
    case 0:
        for (;;) {
            numread = read(STDIN_FILENO, buf, BUF_SIZE);
            if (numread == -1) {
                continue;
            }

            if (write(cfd, msgPrefix, strlen(msgPrefix)) != strlen(msgPrefix)) {
                errExit("write");
            }
            if (write(cfd, buf, numread) != numread) {
                errExit("write");
            }
        }
    default:
        for (;;) {
            numread = read(cfd, buf, BUF_SIZE);
            if (numread == -1) {
                continue;
            }

            if (write(STDOUT_FILENO, buf, numread) != numread) {
                errExit("write");
            }
        }

    }
}