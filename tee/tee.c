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

    if ((argc != 3 && argc != 2 && argc != 1) || strcmp(argv[1], "--help") == 0)
        usageErr("%s filename\n", argv[0]);

    if (argc == 2){
        openFlags = O_CREAT | O_WRONLY | O_TRUNC | O_APPEND;
        filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                    S_IROTH | S_IWOTH;      /* rw-rw-rw- */
        outputFd = open(argv[1], openFlags, filePerms);
        if (outputFd == -1)
            errExit("opening file %s", argv[1]);
    }
    if (argc == 3){
        openFlags = O_WRONLY | O_APPEND;
        while ((opt = getopt(argc, argv, "a")) != -1){
            switch (opt) {
                case 'a': break; 
                default: fatal("Unexpected option!");
            }
        }
        outputFd = open(argv[2], openFlags);
        if (outputFd == -1)
            errExit("opening file %s", argv[1]);
    }

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0){
        if (write(STDOUT_FILENO, buf, numRead) != numRead)
            fatal("write() returned error or partial write occurred");
        if(argc != 1){
            if (write(outputFd, buf, numRead) != numRead)
                fatal("write() returned error or partial write occurred");
        }
    }

    if (numRead == -1)
        errExit("read");

    if (close(outputFd) == -1)
        errExit("close output");

    exit(EXIT_SUCCESS);
}