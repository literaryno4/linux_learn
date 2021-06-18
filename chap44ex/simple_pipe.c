#include <sys/wait.h>
#include "../include/tlpi_hdr.h"

#define BUF_SIZE 10

int
main(int argc, char *argv[])
{
    int pfd[2];
    char buf[BUF_SIZE];
    ssize_t numRead;

    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("usage");
    }

    if (pipe(pfd) == -1) {
        errExit("pipe");
    }

    switch (fork()) {
    case -1:
        errExit("fork");
    case 0:
        if (close(pfd[1]) == -1) {
            errExit("close");
        }

        for (;;) {
            numRead = read(pfd[0], buf, BUF_SIZE);
            if (numRead == -1) {
                errExit("read");
            }

            if (numRead == 0) {
                break;
            }

            if (write(STDOUT_FILENO, buf, numRead) != numRead) {
                fatal("write");
            }
            write(STDOUT_FILENO, "\n", 1);
            if (close(pfd[0] == -1)) {
                errExit("close");
            }
            _exit(EXIT_SUCCESS);
        }
    default:
        if (close(pfd[0]) == -1) {
            errExit("close");
        }

        if (write(pfd[1], argv[1], strlen(argv[1])) != strlen(argv[1])) {
            fatal("write");
        }

        if (close(pfd[1]) == -1) {
            errExit("close");
        }

        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}