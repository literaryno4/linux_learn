#include <sys/socket.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/tlpi_hdr.h"

#define PORT_NUM "50002"
#define BUF_SIZE 1000
#define MAX_FILE_SIZE 100000

int
do_ls(int cfd)
{
    int flag, numread;
    char response[BUF_SIZE];
    
    flag = 0;
    for (;;) {
        numread = read(cfd, response, BUF_SIZE);
        if (numread == 0 && flag == 1){
            break;
        }
        if (numread == -1 || numread == 0) {
            continue;
        }
        flag = 1;
        if(write(STDOUT_FILENO, response, numread) != numread) {
            return -1;
        }
    }

    return 0;
}

int
do_cd(int cfd, char* req)
{
    if (write(cfd, req, strlen(req)) == -1) {
        return -1;
    }
    do_ls(cfd);

    return 0;
}

int
do_sendfile(int cfd, char *req, char *file)
{
    int fd, numsend, n;
    char response[BUF_SIZE];

    if (write(cfd, req, strlen(req)) == -1) {
        return -1;
    }

    if (read(cfd, response, BUF_SIZE) == -1) {
        return -1;
    }

    if ((fd = open(file, O_RDONLY)) == -1) {
        return -1;
    }

    numsend = 0;
    while((n = sendfile(cfd, fd, NULL, MAX_FILE_SIZE)) > 0) {
        numsend += n;
    }
    // while((numread = read(fd, buf, BUF_SIZE)) > 0) {
    //     write(cfd, buf, numread);
    // }

    printf("sent %d bytes\n", numsend);

    return 0;
}

int
do_recvfile(int cfd, char *req, char *file)
{       
    int fd, numread;
    char response[BUF_SIZE];
    
    if (write(cfd, req, strlen(req)) == -1) {
        return -1;
    }

    fd = open(file, O_CREAT | O_RDWR | O_APPEND, 0664);

    int flag = 0;
    for (;;) {
        numread = read(cfd, response, BUF_SIZE);
        if (numread == 0 && flag == 1){
            break;
        }
        if (numread == -1 || numread == 0) {
            continue;
        }
        flag = 1;
        if(write(fd, response, numread) != numread) {
            close(fd);
            return -1;
        }
    }
    close(fd);

    return 0;
}

int main(int argc, char *argv[])
{
    int cfd, numread;
    char response[BUF_SIZE];
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    if (argc < 3 || argc > 5) {
        usageErr("%s ip reqcode param1 param2", argv[0]);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;
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
            break;
        }

        close(cfd);
    }

    if (rp == NULL) {
        fatal("could not connetct");
    }

    freeaddrinfo(result);

    if (write(cfd, argv[2], strlen(argv[2])) == -1) {
        errExit("write");
    }
    for (;;) {
        numread = read(cfd, response, BUF_SIZE);
        if ( numread ==  -1 || numread == 0) {
            continue;
        }
        response[numread] = '\0';
        printf("server status: %s\n\n", response);
        break;
    }

    if (strcmp(argv[2], "ls") == 0) {
        if (do_ls(cfd) == -1) {
            close(cfd);
            errExit("do_ls");
        }
    } else if (strcmp(argv[2], "cd") == 0) {
        if (do_cd(cfd, argv[3]) == -1) {
            close(cfd);
            errExit("do_cd");
        }
    } else if (strcmp(argv[2], "get") == 0) {
        if (do_recvfile(cfd, argv[3], argv[4]) == -1){
            close(cfd);
            errExit("do_recvfile");
        }
    } else if (strcmp(argv[2], "put") == 0) {
        if (do_sendfile(cfd, argv[4], argv[3]) == -1) {
            close(cfd);
            errExit("do_sendfile");
        }
    } else {
        printf("error\n");
    }

    close(cfd);

    exit(EXIT_SUCCESS);
}
