#define _BSD_SOURCE     /* Get getpass() declaration from <unistd.h> */
#define _XOPEN_SOURCE
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "../include/tlpi_hdr.h"

static char *str2;
static int handled = 0;

static void
handler(int sig)
{
    crypt(str2, "xx");
    handled++;
}

int
main(int argc, char *argv[])
{
    char *cr1;
    int callNum, mismatch;
    struct sigaction sa;

    if (argc != 3){
        usageErr("%s str1 str2\n", argv[0]);
    }

    str2 = argv[2];
    cr1 = strdup(crypt(argv[1], "xx"));

    if (cr1 == NULL){
        errExit("strdup");
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGINT, &sa, NULL) == -1){
        errExit("sigaction");
    }

    for (callNum = 1, mismatch = 0; ; callNum++){
        if (strcmp(crypt(argv[1], "xx"), cr1) != 0){
            mismatch++;
            printf("Mismatch on call %d (mismatch=%d handled=%d)\n", callNum, mismatch, handled);
        }
    }
}