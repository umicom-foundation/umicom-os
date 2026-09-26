/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/init.c
 *
 * PURPOSE:
 *   Bring up a diskless guest and retain independent recovery when a service fails.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#define _GNU_SOURCE
#include "boot.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t UmiOsStopRequested = 0;
static void Stop(int number) { (void)number; UmiOsStopRequested = 1; }
static bool Directory(const char *name, mode_t mode)
{
    if (mkdir(name, mode) == 0) return true;
    if (errno != EEXIST) return false;
    struct stat state;
    return lstat(name, &state) == 0 && S_ISDIR(state.st_mode);
}
static bool Mounts(void)
{
    if (!Directory("/proc", 0555) || !Directory("/sys", 0555) || !Directory("/dev", 0755) ||
        !Directory("/run", 0755) || !Directory("/tmp", 01777)) return false;
    if (mount("proc", "/proc", "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC, NULL) != 0 ||
        mount("sysfs", "/sys", "sysfs", MS_NOSUID | MS_NODEV | MS_NOEXEC, NULL) != 0 ||
        mount("devtmpfs", "/dev", "devtmpfs", MS_NOSUID | MS_NOEXEC, "mode=0755") != 0 ||
        mount("tmpfs", "/run", "tmpfs", MS_NOSUID | MS_NODEV | MS_NOEXEC, "mode=0755,size=16m") != 0 ||
        mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV | MS_NOEXEC, "mode=1777,size=8m") != 0)
        return false;
    return Directory("/run/umicom", 0755) && Directory("/run/umicom/logs", 0700) &&
        Directory("/run/umicom/work", 0700) && chown("/run/umicom/work", 1000, 1000) == 0;
}
static bool ReadText(const char *path, char *text, size_t capacity, size_t *outSize)
{
    int fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC); size_t used = 0;
    if (fd < 0) return false;
    for (;;) {
        ssize_t count = read(fd, text + used, capacity - used);
        if (count < 0 && errno == EINTR) continue;
        if (count < 0) { (void)close(fd); return false; }
        if (count == 0) break;
        used += (size_t)count;
        if (used == capacity) { (void)close(fd); return false; }
    }
    if (close(fd) != 0) return false;
    text[used] = 0; *outSize = used; return true;
}
static void PrintReport(const char *mode, const char *state, size_t planned, size_t completed,
    const char *reason, const char *source)
{
    printf("UMICOM_REPORT_BEGIN\nUMICOM_BOOT_REPORT 1\nmode=%s\nstate=%s\nplanned=%zu\ncompleted=%zu\nreason=%s\nsource=%s\nUMICOM_REPORT_END\n",
        mode, state, planned, completed, reason, source);
    fflush(stdout);
}
int main(void)
{
    if (getpid() != 1 || geteuid() != 0) {
        fputs("Umicom init only runs as PID 1 inside the guest. Use the separate host tests.\n", stderr);
        return 78;
    }
    (void)umask(0022);
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    struct sigaction stop = {0}; stop.sa_handler = Stop; sigemptyset(&stop.sa_mask);
    (void)sigaction(SIGTERM, &stop, NULL); (void)sigaction(SIGINT, &stop, NULL);
    (void)sigaction(SIGHUP, &stop, NULL);
    puts("UMICOM_INIT pid=1");
    bool mounted = Mounts();
    int console = open("/dev/console", O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (console >= 0) {
        (void)dup2(console, 0); (void)dup2(console, 1); (void)dup2(console, 2);
        if (console > 2) (void)close(console);
    }
    char text[UMI_OS_CONFIG_MAX + 1], source[UMI_OS_SOURCE_ID_SIZE] = {0}; size_t length = 0;
    UmiOsBootConfig config = {0}; UmiOsBootOptions options = {0};
    const char *reason = mounted ? "none" : "mount";
    bool optionsOk = ReadText("/proc/cmdline", text, sizeof text, &length) &&
        UmiOsBootOptionsParse(text, length, &options);
    if (!optionsOk && mounted) reason = "configuration";
    bool configOk = ReadText("/etc/umicom/boot.conf", text, sizeof text, &length) &&
        UmiOsBootConfigParse(text, length, &config);
    if (!configOk && mounted) reason = "configuration";
    bool sourceOk = ReadText("/etc/umicom/source-id", text, sizeof text, &length) && length == 65 && text[64] == '\n';
    if (sourceOk) for (size_t i = 0; i < 64; ++i)
        if (!((text[i] >= '0' && text[i] <= '9') || (text[i] >= 'a' && text[i] <= 'f'))) sourceOk = false;
    if (sourceOk) memcpy(source, text, 64);
    else { memset(source, '0', 64); if (mounted) reason = "configuration"; }
    if (configOk && sethostname(config.hostname, strlen(config.hostname)) != 0 && mounted) reason = "configuration";
    const char *mode = options.recovery ? "recovery" : "normal";
    /* Forced recovery never starts a normal service, even when its package is
     * present. Configuration remains useful for reading known service logs. */
    if (options.recovery && mounted && configOk && sourceOk) reason = "requested";
    const char *state = strcmp(reason, "none") == 0 ? "starting" : "recovery";
    size_t completed = 0;
    if (mounted && !UmiOsWriteReport("/run/umicom/boot.report", mode, state, config.count, completed, reason, source)) {
        reason = "report-write"; state = "recovery";
    }
    if (strcmp(state, "starting") == 0) {
        for (size_t i = 0; i < config.count; ++i) {
            const UmiOsService *service = &config.services[config.order[i]];
            char logPath[128];
            int count = snprintf(logPath, sizeof logPath, "/run/umicom/logs/%.31s.log", service->id);
            int logFd = count > 0 && (size_t)count < sizeof logPath
                ? open(logPath, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600) : -1;
            if (logFd < 0) { reason = "service-launch"; state = "recovery"; break; }
            printf("UMICOM_SERVICE start=%s\n", service->id);
            UmiOsServiceResult result = UmiOsServiceRun(service, logFd, true);
            if (close(logFd) != 0 && result.state == UMI_OS_SERVICE_OK) result.state = UMI_OS_SERVICE_LAUNCH;
            printf("UMICOM_SERVICE end=%s outcome=%s exit=%d signal=%d\n", service->id,
                UmiOsServiceReason(result.state), result.exitCode, result.signalNumber);
            if (result.state != UMI_OS_SERVICE_OK) { reason = UmiOsServiceReason(result.state); state = "recovery"; break; }
            ++completed;
            if (!UmiOsWriteReport("/run/umicom/boot.report", mode, state, config.count, completed, reason, source)) {
                reason = "report-write"; state = "recovery"; break;
            }
        }
        if (completed == config.count && strcmp(reason, "none") == 0) state = "ready";
    }
    if (mounted && !UmiOsWriteReport("/run/umicom/boot.report", mode, state, config.count, completed, reason, source)) {
        reason = "report-write"; state = "recovery";
    }
    PrintReport(mode, state, config.count, completed, reason, source);
    if (!options.autopoweroff) UmiOsRecoveryConsole(&config);
    puts("UMICOM_SHUTDOWN");
    sync();
    int command = UmiOsStopRequested == 2 ? RB_AUTOBOOT : RB_POWER_OFF;
    (void)reboot(command);
    /* Returning from PID 1 would panic the kernel. Recovery stays resident if
     * the virtual platform cannot service the power operation. */
    for (;;) { int status; while (waitpid(-1, &status, WNOHANG) > 0) {} sleep(1); }
}
