#include <stdio.h>
#include "../include/read_line.h"
#include "../include/inet_sockets.h"
#include "../include/tlpi_hdr.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define BUF_SIZE 2048

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:79.0) Gecko/20100101 Firefox/79.0\r\n";

void readothhrd(int cfd);

int parseRequest(char *request, char *filename, char *host, char *port);

int proxy(int cfd);

int main(int argc, const char *argv[])
{
    int lfd, cfd;
    socklen_t addrlen;

    lfd = inetListen(argv[1], 50, &addrlen);

    for (;;) {
        cfd = accept(lfd, NULL, NULL);
        if (cfd == -1) {
            printf("error accept\n");
            exit(EXIT_FAILURE);
        }

        printf("connected\n");

        proxy(cfd);
    }

    return 0;
}

int
proxy(int cfd)
{
    int numRead, sfd;
    char request[BUF_SIZE], filename[BUF_SIZE], host[BUF_SIZE],
             port[BUF_SIZE], srequest[BUF_SIZE + 2048], buf[BUF_SIZE];

    if ((numRead = readLine(cfd, request, BUF_SIZE)) <= 0) {
        close(cfd);
        return -1;
    }

    printf("%s\n", request);
    readothhrd(cfd);

    parseRequest(request, filename, host, port);

    // parseRequest("GET http://www.cmu.edu/hub/index.html HTTP/1.1", filename, host, port);

    printf("%s %s %s\n", filename, host, port);

    sfd = inetConnect(host, port, SOCK_STREAM);
    if (sfd == -1) {
        close(cfd);
        return -1;
    }

    printf("connected server\n");

    sprintf(srequest, "GET %s HTTP/1.0\r\n", filename);
    write(sfd, srequest, strlen(srequest));
    sprintf(srequest, "Host: %s\r\n", host);
    write(sfd, srequest, strlen(srequest));
    sprintf(srequest, "%s", user_agent_hdr);
    write(sfd, srequest, strlen(srequest));
    sprintf(srequest, "Connection: close\r\n");
    write(sfd, srequest, strlen(srequest));
    sprintf(srequest, "Proxy-Connection: close\r\n\r\n");
    write(sfd, srequest, strlen(srequest));

    while ((numRead = readLine(sfd, buf, BUF_SIZE)) > 0) {
        if (write(cfd, buf, numRead) != numRead) {
            close(cfd);
            close(sfd);
            return -1;
        }
    }
    
    close(cfd);
    close(sfd);

    return 0;
}

int 
parseRequest(char *request, char *filename, char *host, char *port)
{
    char *ptr;
    char method[BUF_SIZE], url[BUF_SIZE], version[BUF_SIZE];

    sscanf(request, "%s %s %s", method, url, version);

    ptr = strstr(url, "http://");
    if (!ptr) {
        if (strstr(url, "https://")){
            sscanf(url, "https://%s", host);
        } else {
            sscanf(url, "%s", host);
        }
    } else {
        sscanf(url, "http://%s", host);
    }

    ptr = index(host, '/');
    if (!ptr) {
        strcpy(filename, "/");
    } else {
        strcpy(filename, ptr);
        *ptr = '\0';
    }

    ptr = index(host, ':');
    if (!ptr) {
        strcpy(port, "80");
    } else {
        strcpy(port, ptr + 1);
        *ptr = '\0';
    }

    return 0;

}

void
readothhrd(int cfd)
{
    char buf[BUF_SIZE];
    readLine(cfd, buf, BUF_SIZE);
    while(strcmp(buf, "\r\n")) {
        readLine(cfd, buf, BUF_SIZE);
        printf("%s", buf);
    }
    return;
}