/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/platform_check.c
 *
 * PURPOSE:
 *   Check actual guest service identity, resource limits and mounted runtime restrictions.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include "guest_environment.h"
#include <linux/magic.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>

/* The original read-only probe is retained for review. The active probe below
 * also verifies the supervisor restrictions and the mounted runtime contract;
 * reading a file alone cannot establish either property. */
#if 0
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
#endif

/* Observe the guest through descriptor-based Linux APIs, then evaluate the
 * policy shared with host regression tests. Unknown observations fail closed.
 * This probe runs as a packaged service; it never repairs mounts or identities. */
static bool ObserveMount(const char *name, unsigned long expectedType, UmiOsGuestMount *out)
{
    int fd = open(name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) return false;
    struct statfs type;
    struct statvfs flags;
    bool observed = fstatfs(fd, &type) == 0 && fstatvfs(fd, &flags) == 0;
    if (close(fd) != 0) observed = false;
    if (!observed) return false;
    const unsigned long restrictions = ST_NOSUID | ST_NODEV | ST_NOEXEC;
    *out = (UmiOsGuestMount){(unsigned long)type.f_type == expectedType,
        (flags.f_flag & restrictions) == restrictions};
    return true;
}
static UmiOsGuestEnvironment ObserveEnvironment(void)
{
    UmiOsGuestEnvironment e = {0};
    e.uid = (uint64_t)getuid(); e.effectiveUid = (uint64_t)geteuid();
    e.gid = (uint64_t)getgid(); e.effectiveGid = (uint64_t)getegid();
    e.observed |= UMI_OS_GUEST_IDENTITY;
    e.supplementaryGroups = getgroups(0, NULL);
    if (e.supplementaryGroups >= 0) e.observed |= UMI_OS_GUEST_GROUPS;
    e.noNewPrivileges = prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0);
    if (e.noNewPrivileges >= 0) e.observed |= UMI_OS_GUEST_PRIVILEGES;
    struct rlimit core, file, descriptors;
    if (getrlimit(RLIMIT_CORE, &core) == 0 && getrlimit(RLIMIT_FSIZE, &file) == 0 &&
        getrlimit(RLIMIT_NOFILE, &descriptors) == 0) {
        /* The hard limits matter too: a restricted child must not be able to
         * raise its soft limit back above the supervisor's promised bound. */
        e.coreLimit = (uint64_t)core.rlim_cur; e.coreHardLimit = (uint64_t)core.rlim_max;
        e.fileLimit = (uint64_t)file.rlim_cur; e.fileHardLimit = (uint64_t)file.rlim_max;
        e.descriptorLimit = (uint64_t)descriptors.rlim_cur;
        e.descriptorHardLimit = (uint64_t)descriptors.rlim_max;
        e.observed |= UMI_OS_GUEST_LIMITS;
    }
    int work = open("/run/umicom/work", O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (work >= 0) {
        struct stat location, current;
        bool observed = fstat(work, &location) == 0 && stat(".", &current) == 0;
        if (close(work) != 0) observed = false;
        if (observed) {
            e.workUid = (uint64_t)location.st_uid; e.workGid = (uint64_t)location.st_gid;
            e.workMode = (uint32_t)(location.st_mode & 07777U);
            e.workDirectory = S_ISDIR(location.st_mode);
            e.workingDirectoryMatches = location.st_dev == current.st_dev &&
                location.st_ino == current.st_ino;
            e.observed |= UMI_OS_GUEST_WORKSPACE;
        }
    }
    if (ObserveMount("/proc", PROC_SUPER_MAGIC, &e.proc)) e.observed |= UMI_OS_GUEST_PROC;
    if (ObserveMount("/sys", SYSFS_MAGIC, &e.sys)) e.observed |= UMI_OS_GUEST_SYS;
    if (ObserveMount("/run", TMPFS_MAGIC, &e.run)) e.observed |= UMI_OS_GUEST_RUN;
    if (ObserveMount("/tmp", TMPFS_MAGIC, &e.tmp)) e.observed |= UMI_OS_GUEST_TMP;
    return e;
}
int main(void)
{
    UmiOsGuestEnvironment environment = ObserveEnvironment();
    UmiOsGuestProblem problem = UmiOsGuestEnvironmentCheck(&environment);
    if (problem != UMI_OS_GUEST_OK) {
        fprintf(stderr, "Guest environment rejected: %s.\n", UmiOsGuestProblemText(problem));
        return 1;
    }
    struct utsname system;
    if (uname(&system) != 0 || strcmp(system.sysname, "Linux") != 0) return 2;
    const char *files[] = {"/proc/version", "/etc/os-release", "/run/umicom/boot.report"};
    for (size_t i = 0; i < sizeof files / sizeof files[0]; ++i) {
        int fd = open(files[i], O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
        if (fd < 0) return 3;
        char first;
        ssize_t count;
        do { count = read(fd, &first, 1); } while (count < 0 && errno == EINTR);
        int closed = close(fd);
        if (count != 1 || closed != 0) return 4;
    }
    int forbidden = open("/etc/umicom/boot.conf", O_WRONLY | O_CLOEXEC | O_NOFOLLOW);
    if (forbidden >= 0) { (void)close(forbidden); return 6; }
    if (errno != EACCES && errno != EROFS) return 7;
    printf("Kernel: %s %s; service uid=%lu gid=%lu. Identity, limits, workspace, mount and read checks passed.\n",
        system.sysname, system.machine, (unsigned long)getuid(), (unsigned long)getgid());
    return 0;
}
