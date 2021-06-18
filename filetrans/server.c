#include <sys/socket.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "../include/tlpi_hdr.h"

#define PORT_NUM "50002"
#define BUF_SIZE 1000
#define MAX_FILE_SIZE 100000

int
responseReqc(int cfd, char *reqc)
{
    int numread;
    numread = read(cfd, reqc, BUF_SIZE);
    if ( numread == -1) {
        return -1;
    }
    reqc[numread] = '\0';
    if (write(cfd, "ok", 2) == -1) {
        return -1;
    }

    return 0;
}

int
do_ls(int cfd)
{
    dup2(cfd, STDOUT_FILENO);
    // system("ls");
    execlp("ls", "ls", NULL);
    return 0;
}

int
do_cd(int cfd)
{
    int numread;
    char req[BUF_SIZE];

    numread = read(cfd, req, BUF_SIZE);
    if ( numread == -1) {
        return -1;
    }
    req[numread] = '\0';
    chdir(req);
    do_ls(cfd);

    return 0;
}

int
do_sendfile(int cfd)
{
    int fd, numread, numsend, n;
    char req[BUF_SIZE];

    numread = read(cfd, req, BUF_SIZE);
    if ( numread == -1) {
        return -1;
    }
    req[numread] = '\0';

    if ((fd = open(req, O_RDONLY)) == -1) {
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
do_recvfile(int cfd)
{
    int fd, numread;
    char response[BUF_SIZE];
    char file[BUF_SIZE];

    if ((numread = read(cfd, file, BUF_SIZE)) == -1) {
        return -1;
    }

    file[numread] = '\0';

    if (write(cfd, "ok", 2) != 2) {
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
            return -1;
        }
    }

    printf("received file save as %s\n", file);

    close(fd);

    return 0;
}

int main(int argc, char *argv[])
{
    int lfd, cfd, optval;
    char reqc[BUF_SIZE];

    struct sigaction sa;
    struct sockaddr_storage claddr;
    socklen_t addrlen;
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        errExit("sigaction");
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, PORT_NUM, &hints, &result) != 0) {
        errExit("getaddrinfo");
    }

    optval = 1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lfd == -1) {
            continue;
        }

        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                sizeof(optval)) == -1) {
            errExit("setsockopt");
        }

        if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        
        close(lfd);
    }

    if (rp == NULL) {
        fatal("could not bind");
    }

    if (listen(lfd, 50) == -1) {
        errExit("listen");
    }

    freeaddrinfo(result);

    for (;;) {
        cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen);
        if (cfd == -1) {
            errExit("accept");
        }

        switch (fork()) {
        case -1:
            close(cfd);
            break;
        case 0:
            close(lfd);
            printf("connected\n");
            responseReqc(cfd, reqc);
            if (strcmp(reqc, "cd") == 0) {
                if (do_cd(cfd) == -1) {
                    close(cfd);
                    printf("error do cd\n");
                    _exit(EXIT_FAILURE);
                }
            } else if (strcmp(reqc, "ls") == 0) {
                if (do_ls(cfd) == -1) {
                    close(cfd);
                    printf("error do ls\n");
                    _exit(EXIT_FAILURE);
                }
            } else if (strcmp(reqc, "get") == 0) {
                if (do_sendfile(cfd) == -1) {
                    close(cfd);
                    printf("error do get\n");
                    _exit(EXIT_FAILURE);
                }
            } else if (strcmp(reqc, "put") == 0) {
                if (do_recvfile(cfd) == -1) {
                    close(cfd);
                    printf("error do put\n");
                    _exit(EXIT_FAILURE);
                }
            } else {
                printf("error\n");
                write(cfd, "could not handle this request!\n", 32);
            }
            _exit(EXIT_SUCCESS);
        default:
            close(cfd);
            break;
        }
    }
}