#include <sys/stat.h>
#include <fcntl.h>
#include "../include/tlpi_hdr.h"

int
my_dup(int oldFd)
{
    return fcntl(oldFd, F_DUPFD);
}

int
my_dup2(int oldFd, int newFd)
{
    if(oldFd != newFd){
        if(open(newFd, O_RDONLY) != -1){
            close(newFd);
        }
        return fcntl(oldFd, F_DUPFD, newFd);
    }else{
        errno = EBADF;
        return -1;
    }
}

int
main(int argc, char *argv[])
{
    char *testContent = "I Love DINGDING!";
    int outputFd, openFlag, newFd, flags;
    mode_t filePerms;

    openFlag = O_WRONLY;
    filePerms = S_IRUSR | S_IWUSR;
    if((outputFd = open("dup.txt", openFlag)) == -1){
        if((outputFd = open("dup.txt", O_CREAT | openFlag, filePerms)) == -1){
            errExit("opening");
        }
    }
    if((newFd = my_dup2(outputFd, 8)) == -1){
        errExit("dup()");
    }
    printf("newFd: %d\n", newFd);

    if(write(outputFd, testContent, strlen(testContent)) != strlen(testContent)){
        fatal("write()");
    }

    if(write(newFd, testContent, strlen(testContent)) != strlen(testContent)){
        fatal("write()");
    }

    if((flags = fcntl(newFd, F_GETFL)) != -1){
        printf("newFd flags: %d\n", flags);
    }else{
        errExit("fcntl");
    }

    if((flags = fcntl(outputFd, F_GETFL)) != -1){
        printf("outputFd flags: %d\n", flags);
        fcntl(outputFd, F_SETFL, flags | O_APPEND);
    }else{
        errExit("fcntl");
    }

    if((flags = fcntl(newFd, F_GETFL)) != -1){
        printf("newFd flags: %d\n", flags);
    }else{
        errExit("fcntl");
    }

    exit(EXIT_SUCCESS);
}