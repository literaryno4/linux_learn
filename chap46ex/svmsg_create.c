#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include "../include/tlpi_hdr.h"

int 
main(int argc, char *argv[])
{
    int numKeyFlags;
    int flags, msqid, opt;
    unsigned int perms;
    long lkey;
    key_t key;

    numKeyFlags = 0;
    flags = 0;

    while((opt = getopt(argc, argv, "cf:k:px")) != -1) {
        switch (opt) {
        case 'c':
            flags |= IPC_CREAT;
            break;
        case 'f':
            key = ftok(optarg, 1);
            if (key == -1) {
                errExit("ftok");
            }
            numKeyFlags++;
            break;
        case 'k':
            if (sscanf(optarg, "%li", &lkey) != 1) {
                cmdLineErr("-k option");
            }

            key = lkey;
            numKeyFlags++;
            break;
        case 'p':
            key = IPC_PRIVATE;
            numKeyFlags++;
            break;
        case 'x':
            flags |= IPC_EXCL;
            break;
        default:
            usageErr("bad option");
        }
    }

    if (numKeyFlags != 1) {
        usageErr("option");
    }

    perms = (optind == argc) ? (S_IRUSR | S_IWUSR) : 
                getInt(argv[optind], GN_BASE_8, "octal-perms");
    
    msqid = msgget(key, flags | perms);
    if (msqid == -1) {
        errExit("msgget");
    }

    printf("%d\n", msqid);
    exit(EXIT_SUCCESS);
}