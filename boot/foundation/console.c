/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/console.c
 *
 * PURPOSE:
 *   Provide read-only recovery inspection without a shell or a Framework dependency.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#define _POSIX_C_SOURCE 200809L
#include "boot.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
extern volatile sig_atomic_t UmiOsStopRequested;
static void Show(const char *path)
{
    int fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC); char bytes[512]; size_t shown = 0;
    if (fd < 0) { puts("No record is available for this boot."); return; }
    while (shown < UMI_OS_LOG_MAX) {
        ssize_t n = read(fd, bytes, sizeof bytes);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) break;
        for (ssize_t i = 0; i < n && shown < UMI_OS_LOG_MAX; ++i, ++shown) {
            unsigned char c = (unsigned char)bytes[i];
            putchar((c >= 32U && c <= 126U) || c == '\n' || c == '\t' ? (int)c : '?');
        }
    }
    (void)close(fd); putchar('\n');
}
static void Command(const char *line, const UmiOsBootConfig *config)
{
    if (strcmp(line, "help") == 0) puts("help | status | packages | log SERVICE | poweroff | reboot\nNo shell, disk installer, network or service restart is provided.");
    else if (strcmp(line, "status") == 0) Show("/run/umicom/boot.report");
    else if (strcmp(line, "packages") == 0) Show("/usr/share/umicom/packages.json");
    else if (strcmp(line, "poweroff") == 0) UmiOsStopRequested = 1;
    else if (strcmp(line, "reboot") == 0) UmiOsStopRequested = 2;
    else if (strncmp(line, "log ", 4) == 0) {
        for (size_t i = 0; i < config->count; ++i) if (strcmp(line + 4, config->services[i].id) == 0) {
            char path[128];
            (void)snprintf(path, sizeof path, "/run/umicom/logs/%.31s.log", config->services[i].id);
            Show(path); return;
        }
        puts("Use a service identifier listed in boot.conf, not a path.");
    } else if (line[0]) puts("Unknown command. Type help.");
}
void UmiOsRecoveryConsole(const UmiOsBootConfig *config)
{
    char line[96]; size_t used = 0; bool overflow = false, invalid = false;
    puts("Umicom OS Foundation console. Type help.\nChanges in this RAM-only guest disappear at shutdown.");
    fputs("umicom> ", stdout); fflush(stdout);
    while (!UmiOsStopRequested) {
        int status; while (waitpid(-1, &status, WNOHANG) > 0) {}
        struct pollfd input = {STDIN_FILENO, POLLIN, 0};
        int ready = poll(&input, 1, 1000);
        if (ready < 0 && errno == EINTR) continue;
        if (ready <= 0) continue;
        char c; ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n <= 0) { sleep(1); continue; }
        if (c == '\n' || c == '\r') {
            line[used] = 0;
            if (overflow) puts("Command too long; nothing was executed.");
            else if (invalid) puts("Command contains unsupported characters; nothing was executed.");
            else Command(line, config);
            used = 0; overflow = false; invalid = false;
            if (!UmiOsStopRequested) { fputs("umicom> ", stdout); fflush(stdout); }
        } else if ((unsigned char)c >= 32U && (unsigned char)c <= 126U) {
            if (used + 1 < sizeof line && !overflow) line[used++] = c;
            else overflow = true;
        } else invalid = true;
    }
}
