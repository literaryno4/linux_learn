#include <syslog.h>
#include "../include/inet_sockets.h"
#include "../include/tlpi_hdr.h"
#include "../include/become_daemon.h"

#define SERVICE "echo"

#define BUF_SIZE 500

int
main(int argc, char *argv[])
{
    int sfd;
    ssize_t numRead;
    socklen_t addrlen, len;
    struct sockaddr_storage claddr;
    char buf[BUF_SIZE];
    char addrStr[IS_ADDR_STR_LEN];

    if (becomeDaemon(0) == -1) {
        errExit("becomedaeme");
    }

    sfd = inetBind(SERVICE, SOCK_DGRAM, &addrlen);
    if (sfd == -1) {
        syslog(LOG_ERR, "could not create server socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (;;) {
        len = sizeof(struct sockaddr_storage);
        numRead  = recvfrom(sfd, buf, BUF_SIZE, 0, 
                           (struct sockaddr *) &claddr, &len);
        if (numRead == -1) {
            errExit("recvfrom");
        }

        if (write(sfd,  buf, numRead) != numRead) {
            syslog(LOG_WARNING, "error echoing response to %s (%s)",
                   inetAddressStr((struct sockaddr *) &claddr, len, addrStr, 
                   IS_ADDR_STR_LEN),
                   strerror(errno));
            
        }

        if (sendto(sfd, buf, numRead, 0, (struct sockaddr *) &claddr, len) != numRead) {
            syslog(LOG_WARNING, "error echoing response to %s (%s)",
                   inetAddressStr((struct sockaddr *) &claddr, len, addrStr, IS_ADDR_STR_LEN),
                   strerror(errno));
        }
    }
}
