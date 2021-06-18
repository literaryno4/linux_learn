#include <sys/stat.h>
#include <fcntl.h>
#include "../include/tlpi_hdr.h"

#ifndef BUF_SIZE        /* Allow "cc -D" to override definition */
#define BUF_SIZE 1024
#endif

int
main(int argc, char *argv[])
{
    int outputFd, openFlags, opt;
    mode_t filePerms;
    ssize_t numRead;
    char buf[BUF_SIZE];

    openFlags = O_WRONLY | O_APPEND;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH;      /* rw-rw-rw- */
    outputFd = open("ex2.txt", openFlags);
    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0){
        if (lseek(outputFd, 0, SEEK_SET) == -1){
            errExit("lseek");
        }
        if (write(outputFd, buf, numRead) != numRead)
            fatal("write() returned error or partial write occurred");
    }

    if (numRead == -1)
        errExit("read");

    if (close(outputFd) == -1)
        errExit("close output");

    exit(EXIT_SUCCESS);
}