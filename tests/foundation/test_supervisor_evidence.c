/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tests/foundation/test_supervisor_evidence.c
 *
 * PURPOSE:
 *   Check how the real supervisor interprets failed clock, wait and pipe operations.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#define _POSIX_C_SOURCE 200809L
#include "boot.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

typedef enum Fault {
    CLOCK_START, CLOCK_POLL, CLOCK_BACKWARDS, SLEEP_ERROR, SLEEP_INTERRUPT,
    PIPE_ERROR, PIPE_INTERRUPT, PIPE_SHORT, WAIT_REAP
} Fault;

static Fault fault;
static unsigned int clockCalls;
static unsigned int injected;

int __real_clock_gettime(clockid_t clock, struct timespec *time);
int __real_nanosleep(const struct timespec *duration, struct timespec *remaining);
ssize_t __real_read(int descriptor, void *data, size_t capacity);
pid_t __real_waitpid(pid_t process, int *status, int options);

/* Linker wrappers affect only this host fixture. The production supervisor
 * still forks and waits for a real packaged test child. */
int __wrap_clock_gettime(clockid_t clock, struct timespec *time)
{
    if (clock == CLOCK_MONOTONIC) {
        ++clockCalls;
        if ((fault == CLOCK_START && clockCalls == 1) || (fault == CLOCK_POLL && clockCalls == 2)) {
            ++injected;
            errno = EINVAL;
            return -1;
        }
        /* Two controlled samples exercise the backwards-clock path even on
         * a newly booted host whose real clock may be near zero. */
        if (fault == CLOCK_BACKWARDS && clockCalls <= 2) {
            if (clockCalls == 2) ++injected;
            *time = (struct timespec){.tv_sec = clockCalls == 1 ? 100 : 99};
            return 0;
        }
    }
    return __real_clock_gettime(clock, time);
}

int __wrap_nanosleep(const struct timespec *duration, struct timespec *remaining)
{
    /* The interruption path remains eligible for a real deadline. The other
     * failure makes further time observation unreliable. */
    if ((fault == SLEEP_ERROR || fault == SLEEP_INTERRUPT) && injected == 0) {
        ++injected;
        errno = fault == SLEEP_ERROR ? EINVAL : EINTR;
        return -1;
    }
    return __real_nanosleep(duration, remaining);
}

ssize_t __wrap_read(int descriptor, void *data, size_t capacity)
{
    /* The test driver's only read calls come from the production launch
     * channel. Simulate read failure, an interruption and a truncated record. */
    if ((fault == PIPE_ERROR || fault == PIPE_INTERRUPT) && injected == 0) {
        ++injected;
        errno = fault == PIPE_ERROR ? EIO : EINTR;
        return -1;
    }
    if (fault == PIPE_SHORT && injected == 0 && capacity > 0) {
        ++injected;
        ((unsigned char *)data)[0] = 1;
        return 1;
    }
    return __real_read(descriptor, data, capacity);
}

pid_t __wrap_waitpid(pid_t process, int *status, int options)
{
    pid_t waited = __real_waitpid(process, status, options);
    /* Reap the real child first so the simulated reporting error cannot
     * leave a test process behind on the host. */
    if (fault == WAIT_REAP && process > 0 && waited == process) {
        ++injected;
        errno = ECHILD;
        return -1;
    }
    return waited;
}

#define CHECK(expression) do { \
    if (!(expression)) { \
        fprintf(stderr, "check %d: %s (errno=%d)\n", __LINE__, #expression, errno); \
        return 1; \
    } \
} while (0)

int main(int argc, char **argv)
{
    static const struct Scenario {
        const char *name;
        Fault fault;
        bool runningChild;
        UmiOsServiceState state;
        int error;
    } scenarios[] = {
        {"clock-start", CLOCK_START, false, UMI_OS_SERVICE_LAUNCH, EINVAL},
        {"clock-poll", CLOCK_POLL, true, UMI_OS_SERVICE_LAUNCH, EINVAL},
        {"clock-backwards", CLOCK_BACKWARDS, true, UMI_OS_SERVICE_LAUNCH, EIO},
        {"sleep-error", SLEEP_ERROR, true, UMI_OS_SERVICE_LAUNCH, EINVAL},
        {"sleep-interrupt", SLEEP_INTERRUPT, true, UMI_OS_SERVICE_TIMEOUT, 0},
        {"pipe-error", PIPE_ERROR, false, UMI_OS_SERVICE_LAUNCH, EIO},
        {"pipe-interrupt", PIPE_INTERRUPT, false, UMI_OS_SERVICE_OK, 0},
        {"pipe-short", PIPE_SHORT, false, UMI_OS_SERVICE_LAUNCH, EPROTO},
        {"wait-reap", WAIT_REAP, true, UMI_OS_SERVICE_LAUNCH, ECHILD},
    };
    CHECK(argc == 3 && argv[2][0] == '/');
    const struct Scenario *selected = NULL;
    /* Select exactly one fault per process so injection state cannot leak
     * into another case. Slow children use the existing timeout fixture. */
    for (size_t index = 0; index < sizeof scenarios / sizeof scenarios[0]; ++index)
        if (strcmp(argv[1], scenarios[index].name) == 0) selected = &scenarios[index];
    CHECK(selected != NULL);
    fault = selected->fault;

    char directory[] = "/tmp/umicom-os-supervision-XXXXXX";
    CHECK(mkdtemp(directory) != NULL);
    UmiOsService service = {.timeoutMs = 1000};
    const char *childName = selected->runningChild ? "timeout" : "success";
    if (fault == SLEEP_INTERRUPT || fault == WAIT_REAP) service.timeoutMs = 100;
    int length = snprintf(service.executable, sizeof service.executable, "%s/%s", directory, childName);
    CHECK(length > 0 && (size_t)length < sizeof service.executable);
    CHECK(symlink(argv[2], service.executable) == 0);
    /* Use a writable standard device for logs and remove only the symlink
     * and uniquely created directory owned by this test. */
    int log = open("/dev/null", O_WRONLY | O_CLOEXEC);
    CHECK(log >= 0);
    UmiOsServiceResult result = UmiOsServiceRun(&service, log, false);
    CHECK(close(log) == 0);
    CHECK(unlink(service.executable) == 0);
    CHECK(rmdir(directory) == 0);

    /* Compare corroborating evidence as well as the high-level outcome. */
    CHECK(injected == 1);
    CHECK(result.state == selected->state && result.launchError == selected->error);
    if (fault == PIPE_SHORT || fault == PIPE_ERROR || fault == PIPE_INTERRUPT)
        CHECK(result.exitCode == 0 && result.signalNumber == 0);
    if (fault == CLOCK_POLL || fault == CLOCK_BACKWARDS || fault == SLEEP_ERROR || fault == SLEEP_INTERRUPT)
        CHECK(result.exitCode == -1 && result.signalNumber == SIGKILL);
    if (fault == CLOCK_START || fault == WAIT_REAP)
        CHECK(result.exitCode == -1 && result.signalNumber == 0);
    CHECK(waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD);
    return 0;
}
