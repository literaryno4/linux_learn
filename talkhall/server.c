#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>
#include "../include/tlpi_hdr.h"

#define BACKLOG 50
#define PORT_NUM "50000"
#define MAX_EVENT 5
#define MAX_BUF 1000
#define MAXOPENFDS 10

int main(int argc, char *argv[])
{
    int lfd, cfd, epfd, ready, numOpenFds, numread, j, k;
    char buf[MAX_BUF];
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    struct epoll_event ev;
    struct epoll_event evlist[MAX_EVENT];
    struct sockaddr_storage claddr;
    socklen_t addrlen;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

    if (getaddrinfo(NULL, PORT_NUM, &hints, &result) != 0){
        errExit("getaddrinfo");
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lfd == -1) {
            continue;
        }

        if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }

        close(lfd);
    }

    if (rp == NULL) {
        fatal("could not bind socket to any address");
    }

    if (listen(lfd, BACKLOG) == -1) {
        errExit("listen");
    }

    freeaddrinfo(result);
    
    epfd = epoll_create(MAXOPENFDS);
    if (epfd == -1) {
        errExit("epoll_create");
    }

    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) == -1) {
        errExit("epoll_ctl");
    }

    numOpenFds = 0;
    while (1) {
        ready = epoll_wait(epfd, evlist, MAX_EVENT, -1);
        if (ready == -1) {
            errExit("epoll_wait()");
        }


        for (j = 0; j < ready; j++) {
            if (evlist[j].data.fd == lfd && (evlist[j].events & EPOLLIN)) {
                addrlen = sizeof(struct sockaddr_storage);
                cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen);
                if (cfd == -1) {
                    errExit("accept");
                }
                numOpenFds += 1;
                printf("connected, now %d clients\n", numOpenFds);
                ev.events = EPOLLIN | EPOLLOUT;
                ev.data.fd = cfd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) == -1) {
                    errExit("epoll_ctl");
                }
            } else if (evlist[j].events & EPOLLIN) {
                numread = read(evlist[j].data.fd, buf, MAX_BUF);
                if (numread == -1) {
                    errExit("read");
                }
                for (k = 0; k < ready; k++) {
                    if (evlist[k].events & EPOLLOUT) {
                        if (k == j) {
                            continue;
                        }
                        if (write(evlist[k].data.fd, buf, numread) != numread) {
                            errExit("write");
                        }
                    }
                }
            } else if (evlist[j].events & EPOLLERR) {
                printf("epollerr, closing fd %d\n", evlist[j].data.fd);
                if (close(evlist[j].data.fd) == -1) {
                    errExit("close");
                }
                numOpenFds--;
            }
        }
    }

}