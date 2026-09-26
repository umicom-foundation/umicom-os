/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/supervisor.c
 *
 * PURPOSE:
 *   Run a packaged one-shot service with a deadline and a private process group.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include "boot.h"
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

typedef struct LaunchError { int stage; int number; } LaunchError;
static uint64_t Now(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0 || now.tv_sec < 0) return UINT64_MAX;
    return (uint64_t)now.tv_sec * UINT64_C(1000) + (uint64_t)now.tv_nsec / UINT64_C(1000000);
}
static void ChildFail(int fd, int stage)
{
    LaunchError error = {stage, errno};
    const char *p = (const char *)&error; size_t left = sizeof error;
    while (left) {
        ssize_t n = write(fd, p, left);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) break;
        p += n; left -= (size_t)n;
    }
    _exit(126);
}
static void Limit(int resource, rlim_t value, int fd)
{
    struct rlimit limit = {value, value};
    /* glibc's GNU declaration uses an enum; musl's declaration uses int.
     * Only the named RLIMIT constants below reach this private adapter. */
#ifdef __GLIBC__
    if (setrlimit((__rlimit_resource_t)resource, &limit) != 0) ChildFail(fd, 2);
#else
    if (setrlimit(resource, &limit) != 0) ChildFail(fd, 2);
#endif
}
static void RunChild(const UmiOsService *service, int logFd, int errorFd, bool dropIdentity)
{
    if (setpgid(0, 0) != 0) ChildFail(errorFd, 1);
    sigset_t empty;
    if (sigemptyset(&empty) != 0 || sigprocmask(SIG_SETMASK, &empty, NULL) != 0)
        ChildFail(errorFd, 1);
    const int signals[] = {SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGCHLD, SIGPIPE, SIGALRM};
    struct sigaction action = {0}; action.sa_handler = SIG_DFL;
    if (sigemptyset(&action.sa_mask) != 0) ChildFail(errorFd, 1);
    for (size_t i = 0; i < sizeof signals / sizeof signals[0]; ++i)
        if (sigaction(signals[i], &action, NULL) != 0) ChildFail(errorFd, 1);
    if (dup2(logFd, STDOUT_FILENO) < 0 || dup2(logFd, STDERR_FILENO) < 0)
        ChildFail(errorFd, 1);
    int input = open("/dev/null", O_RDONLY | O_CLOEXEC);
    if (input < 0 || dup2(input, STDIN_FILENO) < 0) ChildFail(errorFd, 1);
    if (input > STDERR_FILENO) (void)close(input);
    long originalMaximum = sysconf(_SC_OPEN_MAX);
    if (originalMaximum < 0) ChildFail(errorFd, 1);
    Limit(RLIMIT_CORE, 0, errorFd);
    Limit(RLIMIT_FSIZE, UMI_OS_LOG_MAX, errorFd);
    Limit(RLIMIT_NOFILE, 32, errorFd);
    Limit(RLIMIT_CPU, (rlim_t)(service->timeoutMs / 1000U + 2U), errorFd);
    if (dropIdentity) {
        Limit(RLIMIT_AS, 96U * 1024U * 1024U, errorFd);
        Limit(RLIMIT_NPROC, 32, errorFd);
        if (setgroups(0, NULL) != 0 || setgid(1000) != 0 || setuid(1000) != 0 ||
            getuid() != 1000 || geteuid() != 1000 || chdir("/run/umicom/work") != 0)
            ChildFail(errorFd, 2);
    }
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) ChildFail(errorFd, 2);
    /* All descriptors in this private launcher originate here. close_range is
     * intentionally not required, so the image also works on older LTS hosts. */
    bool closed = false;
#ifdef SYS_close_range
    bool lower = errorFd <= 3 || syscall(SYS_close_range, 3U, (unsigned)(errorFd - 1), 0U) == 0;
    bool upper = syscall(SYS_close_range, (unsigned)(errorFd + 1), ~0U, 0U) == 0;
    closed = lower && upper;
#endif
    if (!closed) for (int fd = 3; (long)fd < originalMaximum; ++fd)
        if (fd != errorFd) (void)close(fd);
    char *const arguments[] = {(char *)service->executable, NULL};
    char *const environment[] = {"PATH=/usr/bin:/bin", "HOME=/run/umicom/work", "LANG=C", NULL};
    execve(service->executable, arguments, environment);
    ChildFail(errorFd, 1);
}
UmiOsServiceResult UmiOsServiceRun(const UmiOsService *service, int logFd, bool dropIdentity)
{
    UmiOsServiceResult result = {UMI_OS_SERVICE_LAUNCH, -1, 0, EINVAL, 0};
    int pipeFd[2]; int waitStatus = 0; uint64_t start;
    if (!service || logFd < 0 || !memchr(service->executable, 0, sizeof service->executable) ||
        service->executable[0] != '/' || service->timeoutMs < 100U || service->timeoutMs > 30000U)
        return result;
    start = Now();
    if (start == UINT64_MAX) { result.launchError = errno; return result; }
    if (pipe2(pipeFd, O_CLOEXEC | O_NONBLOCK) != 0) { result.launchError = errno; return result; }
    pid_t pid = fork();
    if (pid == 0) { (void)close(pipeFd[0]); RunChild(service, logFd, pipeFd[1], dropIdentity); _exit(126); }
    if (pid < 0) { result.launchError = errno; (void)close(pipeFd[0]); (void)close(pipeFd[1]); return result; }
    (void)close(pipeFd[1]);
    (void)setpgid(pid, pid); /* The child checks its own result before exec. */
    result.launchError = 0;
    bool timedOut = false, waitFailed = false;
    for (;;) {
        siginfo_t information = {0};
        if (waitid(P_PID, (id_t)pid, &information, WEXITED | WNOHANG | WNOWAIT) != 0) {
            if (errno == EINTR) continue;
            result.launchError = errno; waitFailed = true; break;
        }
        if (information.si_pid == pid) break;
        uint64_t now = Now();
        if (now == UINT64_MAX || now - start >= service->timeoutMs) { timedOut = true; break; }
        struct timespec delay = {0, 10000000L};
        while (nanosleep(&delay, &delay) != 0 && errno == EINTR) {}
    }
    /* Keep the direct child unreaped until its group is terminated. Its PID
     * cannot be recycled into an unrelated process group during this cleanup.
     * This contains ordinary descendants, not hostile setsid/fork behaviour;
     * packaged services remain trusted, and this is not a code sandbox. */
    (void)kill(-pid, SIGKILL);
    (void)kill(pid, SIGKILL);
    pid_t waited;
    do { waited = waitpid(pid, &waitStatus, 0); } while (waited < 0 && errno == EINTR);
    LaunchError error = {0};
    char *bytes = (char *)&error; size_t have = 0;
    while (have < sizeof error) {
        ssize_t n = read(pipeFd[0], bytes + have, sizeof error - have);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) break;
        have += (size_t)n;
    }
    (void)close(pipeFd[0]);
    uint64_t end = Now(); result.elapsedMs = end == UINT64_MAX ? 0U : end - start;
    if (timedOut) result.state = UMI_OS_SERVICE_TIMEOUT;
    else if (waitFailed || waited != pid || (have != 0 && have != sizeof error)) result.state = UMI_OS_SERVICE_LAUNCH;
    else if (have == sizeof error) {
        result.launchError = error.number;
        result.state = error.stage == 2 ? UMI_OS_SERVICE_PRIVILEGE : UMI_OS_SERVICE_LAUNCH;
    } else if (WIFEXITED(waitStatus) && WEXITSTATUS(waitStatus) == 0) result.state = UMI_OS_SERVICE_OK;
    else result.state = UMI_OS_SERVICE_EXIT;
    if (waited == pid) {
        if (WIFEXITED(waitStatus)) result.exitCode = WEXITSTATUS(waitStatus);
        if (WIFSIGNALED(waitStatus)) result.signalNumber = WTERMSIG(waitStatus);
    }
    return result;
}
const char *UmiOsServiceReason(UmiOsServiceState state)
{
    switch (state) {
    case UMI_OS_SERVICE_OK: return "none";
    case UMI_OS_SERVICE_EXIT: return "service-exit";
    case UMI_OS_SERVICE_TIMEOUT: return "service-timeout";
    case UMI_OS_SERVICE_PRIVILEGE: return "privilege";
    case UMI_OS_SERVICE_LAUNCH: default: return "service-launch";
    }
}
