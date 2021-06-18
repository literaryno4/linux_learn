#include <sys/un.h>
#include <sys/socket.h>
#include "../include/tlpi_hdr.h"

#define SV_SOCK_PATH "/tmp/us_xfr"

#define BUF_SIZE 100

int
main(int argc, char const *argv[])
{
    struct sockaddr_un addr;
    int sfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        errExit("socket");
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) -1);

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        errExit("connetct");
    }

    if (read(sfd, buf, 9) == -1) {
        errExit("read");
    }

    if (write(STDOUT_FILENO, buf, 9) == -1) {
        errExit("write");
    }

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        if (write(sfd, buf, numRead) != numRead) {
            fatal("write");
        }
    }

    if (numRead == -1) {
        errExit("read");
    }

    exit(EXIT_SUCCESS);
}
