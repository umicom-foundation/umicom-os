/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/platform_check.c
 *
 * PURPOSE:
 *   Check the mounted guest environment as the unprivileged service identity.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
int main(void)
{
    struct utsname system; struct stat path;
    if (getuid() != 1000 || geteuid() != 1000 || getgid() != 1000) return 1;
    if (uname(&system) != 0 || strcmp(system.sysname, "Linux") != 0) return 2;
    const char *files[] = {"/proc/version", "/etc/os-release", "/run/umicom/boot.report"};
    for (size_t i = 0; i < sizeof files / sizeof files[0]; ++i) {
        int fd = open(files[i], O_RDONLY | O_CLOEXEC);
        if (fd < 0) return 3;
        char first;
        ssize_t count = read(fd, &first, 1);
        int closed = close(fd);
        if (count != 1 || closed != 0) return 4;
    }
    if (stat("/run/umicom/work", &path) != 0 || path.st_uid != 1000 || !S_ISDIR(path.st_mode)) return 5;
    int forbidden = open("/etc/umicom/boot.conf", O_WRONLY | O_CLOEXEC);
    if (forbidden >= 0) { (void)close(forbidden); return 6; }
    if (errno != EACCES && errno != EROFS) return 7;
    printf("Kernel: %s %s; service uid=%lu gid=%lu. Read checks passed.\n",
        system.sysname, system.machine, (unsigned long)getuid(), (unsigned long)getgid());
    return 0;
}
