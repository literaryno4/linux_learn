#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include "../include/tlpi_hdr.h"

#define BUF_SIZE 10

#define PORT_NUM 50002

int
main(int argc, char *argv[])
{
    struct sockaddr_in6 svaddr;
    int sfd, j;
    size_t msgLen;
    ssize_t numBytes;
    char resp[BUF_SIZE];

    if (argc < 3) {
        usageErr("usage");
    }

    sfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sfd == -1) {
        errExit("socket");
    }

    memset(&svaddr, 0, sizeof(struct sockaddr_in6));
    svaddr.sin6_family = AF_INET6;
    svaddr.sin6_port = htons(PORT_NUM);
    if (inet_pton(AF_INET6, argv[1], &svaddr.sin6_addr) <= 0) {
        fatal("inte_pton!");
    }

    for (j = 2; j < argc; j++) {
        msgLen = strlen(argv[j]);
        if (sendto(sfd, argv[j], msgLen, 0, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_in6)) != msgLen) {
            fatal("sendto");
        }

        numBytes = recvfrom(sfd, resp, BUF_SIZE, 0, NULL, NULL);
        if (numBytes == -1) {
            errExit("recvfrom");
        }

        printf("response %d: %.*s\n", j - 1, (int) numBytes, resp);
    }
    exit(EXIT_SUCCESS);
}