#define _BSD_SOURCE
#define _XOPEN_SOURCE
#include <sys/capability.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#include <shadow.h>
#include "../include/tlpi_hdr.h"

static int
modifyCap(int capability, int setting)
{
    cap_t caps;
    cap_value_t capList[1];

    caps = cap_get_proc();
    if (caps == NULL) {
        return -1;
    }

    capList[0] = capability;
    if (cap_set_flag(caps, CAP_EFFECTIVE, 1, capList, setting) == -1) {
        cap_free(caps);
        return -1;
    }

    if (cap_set_proc(caps) == -1) {
        cap_free(caps);
        return -1;
    }

    if (cap_free(caps) == -1) {
        return -1;
    }

    return 0;
}

static int
raiseCap(int capability)
{
    return modifyCap(capability, CAP_SET);
}

static int
dropAllCaps(void)
{
    cap_t empty;
    int s;

    empty = cap_init();
    if (empty == NULL) {
        return -1;
    }

    s = cap_set_proc(empty);
    if (cap_free(empty) == -1) {
        return -1;
    }

    return s;
}

int
main(int argc, char *argv[])
{
    char *username, *password, *encrypted, *p;
    struct passwd *pwd;
    struct spwd *spwd;
    Boolean authOk;
    size_t len;
    long lnmax;

    lnmax = sysconf(_SC_LOGIN_NAME_MAX);
    if (lnmax == -1) {
        lnmax = 256;
    }

    username = malloc(lnmax);
    if (username == NULL) {
        errExit("malloc");
    }

    printf("Username: ");
    fflush(stdout);
    if (fgets(username, lnmax, stdin) == NULL) {
        exit(EXIT_FAILURE);
    }

    len = strlen(username);
    if (username[len - 1] == '\n') {
        username[len - 1] = '\0';
    }

    pwd = getpwnam(username);
    if (pwd == NULL) {
        fatal("could not get pwd record");
    }

    if (raiseCap(CAP_DAC_READ_SEARCH) == -1) {
        fatal("raiseCap");
    }

    spwd = getspnam(username);

    if (spwd == NULL && errno == EACCES) {
        fatal("no permission");
    }

    if (dropAllCaps() == -1) {
        fatal("dropcaps");
    }

    if (spwd != NULL) {
        pwd->pw_passwd = spwd->sp_pwdp;
    }

    password = getpass("Password: ");

    encrypted = crypt(password, pwd->pw_passwd);
    for (p = password; *p != '\0';) {
        *p++ = '\0';
    }

    if (encrypted == NULL) {
        errExit("crypt");
    }

    authOk = strcmp(encrypted, pwd->pw_passwd) == 0;

    if (!authOk) {
        printf("Incorrect password\n");
        exit(EXIT_FAILURE);
    }

    printf("authenticated: UID=%ld\n", (long) pwd->pw_uid);

    exit(EXIT_SUCCESS);
}