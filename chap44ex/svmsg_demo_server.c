#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/tlpi_hdr.h"

#define KEY_FILE "/tmp/kye_file"

int
main(int argc, char *argv[])
{
    int msqid;
    key_t key;
    const int MQ_PERMS = S_IRUSR | S_IWGRP | S_IWGRP;
    if (open(KEY_FILE, O_CREAT | O_RDWR) == -1) {
        errExit("create");
    }
    key = ftok(KEY_FILE, 1);
    printf("%d", (int) key);
    if (key == -1) {
        errExit("ftok");
    }

    while ((msqid = msgget(key, IPC_CREAT | IPC_EXCL | MQ_PERMS)) == -1){
        if (errno == EEXIST) {
            msqid = msgget(key, 0);
            if (msqid == -1) {
                errExit("msgget() failed to retrieve old queue ID");
            }
            if (msgctl(msqid, IPC_RMID, NULL) == -1) {
                errExit("msgget() failed to delete old queue");
            }
        } else {
            errExit("msgget() failed");
        }
    }

    exit(EXIT_SUCCESS);
}