#include <pthread.h>
#include "../include/tlpi_hdr.h"

static int glob = 0;

static void *
threadFunc(void *arg)
{
    int loops = *((int *) arg);
    int loc, j;

    for (j = 0; j < loops; j++) {
        glob++;
    }
    return NULL;
}

int
main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int loops, s;

    loops = (argc > 1) ? getInt(argv[1], GN_GT_0, "num-loops") : 10000000;

    s = pthread_create(&t1, NULL, threadFunc, &loops);
    if (s != 0) {
        errExitEN(s, "pthread_create");
    }
    s = pthread_create(&t2, NULL, threadFunc, &loops);
    if (s != 0) {
        errExitEN(s, "pthread_create");
    }

    s = pthread_join(t1, NULL);
    if (s != 0) {
        errExitEN(s, "pthread_join");
    }
    s = pthread_join(t2, NULL);
    if (s != 0) {
        errExitEN(s, "pthread_join");
    }

    printf("glob = %d\n", glob);
    exit(EXIT_SUCCESS);
}