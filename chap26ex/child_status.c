#include <sys/wait.h>
#include "../include/print_wait_status.h"
#include "../include/tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int status;
    pid_t childPid;

    if (argc > 1 && strcmp(argv[1], "--help") == 0){
        usageErr("%s [exit-status]\n", argv[0]);
    }

    switch (fork()) {
    case -1:
        errExit("fork");
    case 0:
        printf("Child started with PID = %ld\n", (long) getpid());
        if (argc > 1) {
            exit(getInt(argv[1], 0, "exit-status"));
        } else {
            for (;;) {
                pause();
            }
        }
        exit(EXIT_FAILURE);
    default:
        for (;;) {
            childPid = waitpid(-1, &status, WUNTRACED
            #ifdef WCONTINUED
                | WCONTINUED
            #endif
                );
            
            if (childPid == -1) {
                errExit("waitpid");
            }
            printf("waitpid() returned: PID=%ld; status=0x%04x (%d, %d)\n",
                    (long) childPid,
                    (unsigned int) status, status >> 8, status & 0xff);
            printWaitStatus(NULL, status);

            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                exit(EXIT_SUCCESS);
            }
        }

    }
}