#include <syslog.h>
#include "../include/tlpi_hdr.h"

int
main (int argc, const char *argv[])
{
    char buf[4096];
    ssize_t numread;

    while ((numread = read(STDIN_FILENO, buf, 4096)) > 0) {
        if (write(STDOUT_FILENO, buf, numread) != numread) {
            syslog(LOG_ERR, "write");
            exit(EXIT_FAILURE);
        }
    }

    if (numread == -1) {
        syslog(LOG_ERR, "read");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}