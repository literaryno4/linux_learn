#include "../include/tlpi_hdr.h"

static int idata = 111;

int
main(int argc, char *argv[])
{
    int istack = 222;
    pid_t childPid;

    setbuf(stdout, NULL);

    switch (childPid = fork())
    {
    case -1:
        errExit("fork");
    case 0:
        idata *= 3;
        istack *= 3;
        sleep(3);
        printf("hhh\n");
        printf("ppid: %ld\n", (long) getppid());
        write(STDOUT_FILENO, "yahaha\n", 8);
        _exit(EXIT_SUCCESS);
    default:
        // sleep(3);
        break;
    }
    printf("PID=%ld %s idata=%d istack=%d\n", (long) getpid(), 
            (childPid == 0) ? "child)" : "(parent)", idata, istack);
    
    // exit(EXIT_SUCCESS);
}