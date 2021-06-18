#include <signal.h>
#include <sys/wait.h>
#include "../include/print_wait_status.h"
#include "../include/curr_time.h"
#include "../include/tlpi_hdr.h"

static volatile int numLiveChildren = 0;

static void
sigchldHandler(int sig)
{
    int status, savedErrno;
    pid_t childPid;

    savedErrno = errno;
    printf("%s handler: Caught SIGCHLD\n", currTime("%T"));
    while ((childPid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("%s handler: Reaped child %ld - ", currTime("%T"), (long) childPid);
        printWaitStatus(NULL, status);
        numLiveChildren--;
    }
    
    if (childPid == -1 && errno != ECHILD) {
        errMsg("waitpid");
    }
    sleep(5);
    printf("%s handler: returning\n", currTime("%T"));

    errno = savedErrno;
}

int
main(int argc, char *argv[])
{
    int j, sigCnt;
    sigset_t blockMask, emptyMask;
    struct sigaction sa;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s child-sleep-time...\n", argv[0]);
    }

    setbuf(stdout, NULL);

    sigCnt = 0;
    numLiveChildren = argc - 1;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigchldHandler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        errExit("sigaction");
    }

    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGCHLD);
    if (sigprocmask(SIG_SETMASK, &blockMask, NULL) == -1){
        errExit("sigpromask");
    }

    for (j = 1; j < argc; j++) {
        switch (fork()) {
        case -1:
            errExit("fork");
        case 0:
            sleep(getInt(argv[j], GN_NONNEG, "child-sleep-time"));
            printf("%s Child %d (PID=%ld) exiting\n", currTime("%T"), j, (long) getpid());
            _exit(EXIT_SUCCESS);
        default:
            break;
        }
    }

    sigemptyset(&emptyMask);
    while (numLiveChildren > 0) {
        if (sigsuspend(&emptyMask) == -1 && errno != EINTR) {
            errExit("sigsuspend");
        }
        sigCnt++;
    }

    printf("%s All %d children have terminated; SIGCHLD was caught "
            "%d times\n", currTime("%T"), argc - 1, sigCnt);
    exit(EXIT_SUCCESS);
}