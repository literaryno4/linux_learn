#define _GNU_SOURCE
#include <time.h>
#include <utmpx.h>
#include <paths.h>
#include "../include/tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct utmpx *ut;
    
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usageErr("usage");
    }

    if (argc > 1) {
        if (utmpxname(argv[1]) == -1) {
            errExit("utmpxname");
        }
    }

    setutxent();

    printf("user    type    PID line    id  host    date/time\n");

    while ((ut = getutxent()) != NULL) {
        printf("%-8s ", ut->ut_user);
        printf("%-9.9s ",
                (ut->ut_type == EMPTY) ? "EMPTY" :
                (ut->ut_type == RUN_LVL) ? "RUN_LVL" :
                (ut->ut_type == BOOT_TIME) ? "BOOT_TIME" :
                (ut->ut_type == NEW_TIME) ? "NEW_TIME" :
                (ut->ut_type == OLD_TIME) ? "OLD_TIME" :
                (ut->ut_type == INIT_PROCESS) ? "INIT_PROCESS" :
                (ut->ut_type == LOGIN_PROCESS) ? "LOGIN_PROCESS" :
                (ut->ut_type == USER_PROCESS) ? "USER_PROCESS" :
                (ut->ut_type == DEAD_PROCESS) ? "DEAD_PROCESS" : "???");
        printf("%5ld %-6.6s %-3.5s %-9.9s ", (long) ut->ut_pid, ut->ut_line, ut->ut_id, ut->ut_host);
        printf("%s", ctime((time_t *) &(ut->ut_tv.tv_sec)));
    }
    endutxent();
    exit(EXIT_SUCCESS);
}