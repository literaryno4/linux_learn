 #include <sys/epoll.h>
 #include <fcntl.h>
 #include "../include/tlpi_hdr.h"

 #define MAX_BUF 1000
 #define MAX_EVENTS 5

 int
 main(int argc, char *argv[])
 {
     int epfd, ready, fd, s, j, numOpenFds;
     struct epoll_event ev;
     struct epoll_event evlist[MAX_EVENTS];
     char buf[MAX_BUF];

     if (argc < 2) {
         usageErr("%s file...\n", argv[0]);
     }

     epfd = epoll_create(argc - 1);
     if (epfd == -1) {
         errExit("epoll");
     }

     for (j = 1; j < argc; j++) {
         fd = open(argv[j], O_RDONLY);
         if (fd == -1) {
             errExit("open");
         }
         printf("opened \" %s \" on fd %d\n", argv[j], fd);
         ev.events = EPOLLIN;
         ev.data.fd = fd;

         if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
             errExit("epoll_ctl");
         }
     }

     numOpenFds = argc - 1;

     while (numOpenFds > 0) {
         printf("about to epoll_wait\n");
         ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
         if (ready == -1) {
             if (errno == EINTR) {
                 continue;
             } else {
                 errExit("epoll_wait");
             }
         }
         printf("Ready: %d\n", ready);

         for (j = 0; j < ready; j++) {
             printf("    fd=%d; events: %s%s%s\n", evlist[j].data.fd,
                      (evlist[j].events & EPOLLIN) ? "EPOLLIN" : "",
                      (evlist[j].events & EPOLLHUP) ? "EPOLLHUP" : "",
                      (evlist[j].events & EPOLLERR) ? "EPOLLERR" : ""
                      );
             if (evlist[j].events & EPOLLIN) {
                 s = read(evlist[j].data.fd, buf, MAX_BUF);
                 if (s == -1) {
                     errExit("read");
                 }
                 printf("    read %d bytes: %.*s\n", s, s, buf);
             } else if (evlist[j].events & (EPOLLHUP | EPOLLERR)) {
                 printf("    closing fd %d\n", evlist[j].data.fd);
                 if (close(evlist[j].data.fd) == -1) {
                     errExit("close");
                 }
                 numOpenFds--;
             } 
         }
     }

     printf("all fds closed\n");
     exit(EXIT_SUCCESS);
 }