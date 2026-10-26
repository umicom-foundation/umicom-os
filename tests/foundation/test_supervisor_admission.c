/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tests/foundation/test_supervisor_admission.c
 *
 * PURPOSE:
 *   Exercise rejected child-wait policies and closed standard descriptors without changing the test runner streams.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#define _GNU_SOURCE
#include "boot.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHECK(value) do { if (!(value)) { \
    fprintf(stderr, "check %d: %s errno=%d\n", __LINE__, #value, errno); return 1; \
} } while (0)

static int ClosedStreams(UmiOsService *service, int log, bool lowLog, bool absent)
{
    /* Close streams only in an isolated probe, leaving the test runner's
     * capture descriptors intact. The supervisor must still report exec
     * failure and provide readable empty stdin to a successfully exec'd child. */
    pid_t probe = fork();
    if (probe < 0) return 1;
    if (probe == 0) {
        (void)close(STDIN_FILENO);
        (void)close(STDOUT_FILENO);
        (void)close(STDERR_FILENO);
        int passedLog = log;
        if (lowLog) {
            if (dup2(log, STDIN_FILENO) != STDIN_FILENO) _exit(2);
            passedLog = STDIN_FILENO;
            (void)close(log);
        }
        UmiOsServiceResult result = UmiOsServiceRun(service, passedLog, false);
        bool ok = absent ? result.state == UMI_OS_SERVICE_LAUNCH &&
            result.launchError == ENOENT : result.state == UMI_OS_SERVICE_OK;
        if (fcntl(passedLog, F_GETFD) < 0) ok = false;
        _exit(ok ? 0 : 1);
    }
    int status = 0;
    pid_t waited;
    do { waited = waitpid(probe, &status, 0); } while (waited < 0 && errno == EINTR);
    return waited == probe && WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
}
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    const char *name = argv[1];
    char directory[] = "/tmp/umicom-os-admission-XXXXXX";
    CHECK(mkdtemp(directory) != NULL);
    CHECK(chdir(directory) == 0);
    UmiOsService service = {0};
    service.timeoutMs = 1500;
    int size = snprintf(service.executable, sizeof service.executable, "%s/stdin", directory);
    CHECK(size > 0 && (size_t)size < sizeof service.executable);
    bool absent = strcmp(name, "closed-streams-exec") == 0;
    if (!absent) CHECK(symlink(argv[2], service.executable) == 0);
    int log = open("service.log", O_WRONLY | O_CREAT | O_EXCL, 0600);
    CHECK(log >= 0);
    int descriptorFlags = fcntl(log, F_GETFD);
    CHECK(descriptorFlags >= 0);

    if (strcmp(name, "ignored-child") == 0 || strcmp(name, "auto-reap") == 0) {
        struct sigaction action = {0}, previous, after;
        action.sa_handler = strcmp(name, "ignored-child") == 0 ? SIG_IGN : SIG_DFL;
        action.sa_flags = strcmp(name, "auto-reap") == 0 ? SA_NOCLDWAIT : 0;
        CHECK(sigemptyset(&action.sa_mask) == 0);
        CHECK(sigaction(SIGCHLD, &action, &previous) == 0);
        UmiOsServiceResult result = UmiOsServiceRun(&service, log, false);
        CHECK(sigaction(SIGCHLD, NULL, &after) == 0);
        CHECK(sigaction(SIGCHLD, &previous, NULL) == 0);
        CHECK(result.state == UMI_OS_SERVICE_LAUNCH && result.launchError == EINVAL);
        CHECK(result.elapsedMs == 0);
        CHECK(after.sa_handler == action.sa_handler);
        CHECK((after.sa_flags & SA_NOCLDWAIT) == (action.sa_flags & SA_NOCLDWAIT));
    } else if (strcmp(name, "readonly-log") == 0) {
        int readonly = open("service.log", O_RDONLY);
        CHECK(readonly >= 0);
        UmiOsServiceResult result = UmiOsServiceRun(&service, readonly, false);
        CHECK(result.state == UMI_OS_SERVICE_LAUNCH && result.launchError == EBADF);
        CHECK(fcntl(readonly, F_GETFD) >= 0);
        CHECK(close(readonly) == 0);
    } else if (strcmp(name, "closed-log") == 0) {
        int closed = dup(log);
        CHECK(closed >= 0 && close(closed) == 0);
        UmiOsServiceResult result = UmiOsServiceRun(&service, closed, false);
        CHECK(result.state == UMI_OS_SERVICE_LAUNCH && result.launchError == EBADF);
    } else if (strcmp(name, "closed-streams") == 0 ||
               strcmp(name, "closed-streams-exec") == 0 ||
               strcmp(name, "low-log") == 0) {
        CHECK(ClosedStreams(&service, log, strcmp(name, "low-log") == 0, absent) == 0);
    } else if (strcmp(name, "caller-log") == 0) {
        UmiOsServiceResult result = UmiOsServiceRun(&service, log, false);
        CHECK(result.state == UMI_OS_SERVICE_OK);
        CHECK(write(log, "parent\n", 7) == 7);
    } else return 2;
    CHECK(fcntl(log, F_GETFD) == descriptorFlags);
    CHECK(close(log) == 0);
    /* Temporary files remain in this private directory as failure evidence.
     * The fixture never changes host users, mounts, /run or global settings. */
    return 0;
}
