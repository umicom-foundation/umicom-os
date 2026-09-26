/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/report.c
 *
 * PURPOSE:
 *   Publish a bounded volatile boot report using an atomic rename.
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

bool UmiOsWriteReport(const char *path, const char *mode, const char *state,
    size_t planned, size_t completed, const char *reason, const char *sourceId)
{
    char temporary[256], text[512];
    if (!path || !mode || !state || !reason || !sourceId || strlen(path) > 240U ||
        planned > UMI_OS_SERVICE_MAX || completed > planned || strlen(mode) > 8U ||
        strlen(state) > 8U || strlen(reason) > 31U || strlen(sourceId) != 64U) return false;
    /* Treat the file as a protocol, not as a formatting template. Recovery
     * must remain readable even when a caller supplies invalid state. */
    bool normal = strcmp(mode, "normal") == 0;
    bool requested = strcmp(mode, "recovery") == 0;
    bool ready = strcmp(state, "ready") == 0;
    bool starting = strcmp(state, "starting") == 0;
    bool recovery = strcmp(state, "recovery") == 0;
    const char *reasons[] = {"none", "requested", "configuration", "mount", "service-exit",
        "service-timeout", "service-launch", "report-write", "privilege"};
    bool knownReason = false;
    for (size_t i = 0; i < sizeof reasons / sizeof reasons[0]; ++i)
        if (strcmp(reason, reasons[i]) == 0) knownReason = true;
    if ((!normal && !requested) || (!ready && !starting && !recovery) || !knownReason ||
        (!recovery && (!normal || strcmp(reason, "none") != 0 || planned == 0)) ||
        (ready && completed != planned) || (recovery && strcmp(reason, "none") == 0) ||
        (requested && (!recovery || completed != 0)) ||
        (normal && strcmp(reason, "requested") == 0)) return false;
    for (size_t i = 0; i < 64; ++i)
        if (!((sourceId[i] >= '0' && sourceId[i] <= '9') || (sourceId[i] >= 'a' && sourceId[i] <= 'f')))
            return false;
    int n = snprintf(temporary, sizeof temporary, "%s.tmp", path);
    if (n < 0 || (size_t)n >= sizeof temporary) return false;
    n = snprintf(text, sizeof text,
        "UMICOM_BOOT_REPORT 1\nmode=%.8s\nstate=%.8s\nplanned=%zu\ncompleted=%zu\nreason=%.31s\nsource=%.64s\n",
        mode, state, planned, completed, reason, sourceId);
    if (n < 0 || (size_t)n >= sizeof text) return false;
    int fd = open(temporary, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0644);
    if (fd < 0) return false;
    size_t done = 0, length = (size_t)n; bool ok = true;
    while (done < length) {
        ssize_t written = write(fd, text + done, length - done);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) { ok = false; break; }
        done += (size_t)written;
    }
    if (close(fd) != 0) ok = false;
    if (ok && rename(temporary, path) == 0) return true;
    /* Only this function's exclusively-created temporary file is removed. */
    (void)unlink(temporary);
    return false;
}
