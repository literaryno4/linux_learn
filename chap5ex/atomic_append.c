#include <sys/stat.h>
#include <fcntl.h>
#include "../include/tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int outputFd, openFlags;
    int num_bytes = getInt(argv[2], 10, "");
    mode_t filePerms;

    if (argc == 4){
        openFlags = O_WRONLY;
    }else{
        openFlags = O_WRONLY | O_APPEND;
    }
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH;      /* rw-rw-rw- */
    outputFd = open(argv[1], openFlags);
    if(outputFd == -1){
        outputFd = open(argv[1], O_CREAT | openFlags, filePerms);
    }

    while(num_bytes-- > 0){
        if(argc == 4){
            if (lseek(outputFd, 0, SEEK_END) == -1){
                errExit("lseek");
            }
        }
        if(write(outputFd, "a", 1) != 1){
                fatal("write() returned error or partial write occurred");
            }
    }

    if (close(outputFd) == -1)
        errExit("close output");

    exit(EXIT_SUCCESS);
}