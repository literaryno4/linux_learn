#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include "../include/tlpi_hdr.h"

static volatile sig_atomic_t gotAlarm = 0;

static void
displayTimes(const char *msg, Boolean includeTimer)
{
    struct itimerval itv;
    static struct timeval start;
    struct timeval curr;
    static int callNum = 0;

    if (callNum ==0){
        if (gettimeofday(&start, NULL) == -1){
            errExit("gettimeofday");
        }
    }

    if (callNum % 20 == 0){
        printf("         Elapse value Iterval\n");
    }

    if (gettimeofday(&curr, NULL) == -1){
        errExit("gettimeofday");
    }

    printf("%-7s %6.2f", msg, curr.tv_sec - start.tv_sec + (curr.tv_usec - start.tv_usec) / 1000000.0);

    if (includeTimer){
        if (getitimer(ITIMER_REAL, &itv) == -1){
            errExit("getitimer");
        }
        printf("  %6.2f  %6.2f", itv.it_value.tv_sec + itv.it_value.tv_usec / 1000000.0,
                                 itv.it_interval.tv_sec + itv.it_interval.tv_usec / 1000000.0);
    }

    printf("\n");
    callNum++;
}

static void
sigalrmhandler(int sig)
{
    gotAlarm = 1;
}

int
main(int argc, char *argv[])
{
    struct itimerval itv;
    clock_t prevClock;
    int maxSigs;
    int sigCnt;
    struct sigaction sa;
    if (argc > 1 && strcmp(argv[1], "--help") == 0){
        usageErr("%s [secs [usecs [int-secs [int-usecs]]]]\n", argv[0]);
    }
    sigCnt = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigalrmhandler;
    if (sigaction(SIGALRM, &sa, NULL) == -1){
        errExit("sigaction");
    }

    maxSigs = (itv.it_interval.tv_sec == 0 && itv.it_interval.tv_usec == 0) ? 1 : 3;

    printf("maxSigs:%d, %ld\n", maxSigs, itv.it_interval.tv_usec);

    displayTimes("START:", FALSE);

    itv.it_value.tv_sec = (argc > 1) ? getLong(argv[1], 0, "secs") : 2;
    itv.it_value.tv_usec = (argc > 2) ? getLong(argv[2], 0, "usecs") : 0;
    itv.it_interval.tv_sec = (argc > 3) ? getLong(argv[3], 0, "int-secs") : 0;
    itv.it_interval.tv_usec = (argc > 4) ? getLong(argv[4], 0, "int-usecs") : 0;

    if (setitimer(ITIMER_REAL, &itv, 0) == -1){
        errExit("settimer");
    }

    prevClock = clock();
    sigCnt = 0;

    for(;;){
        while (((clock() - prevClock) * 10 / CLOCKS_PER_SEC) < 5){
            if (gotAlarm)  {
                gotAlarm = 0;
                displayTimes("ALARM:", TRUE);

                sigCnt++;
                if (sigCnt >= maxSigs) {
                    printf("That's all folks\n");
                    exit(EXIT_SUCCESS);
                }
            }
        }
        
        prevClock = clock();
        displayTimes("Main:", TRUE);
    }
}