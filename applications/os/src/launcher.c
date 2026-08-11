/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/launcher.c
 *
 * PURPOSE:
 *   Implement one product-level Umicom OS user-space service or model.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

/* BEGINNER NOTE:
 * This file keeps one responsibility small and explicit. Read the public
 * structure/function declarations first, then follow the implementation in
 * the matching source file.
 */
#include "umicom/os/launcher.h"

#include <string.h>

void umi_os_launcher_init(UmiOsLauncher *launcher)
{
    if (launcher != NULL) {
        (void)memset(launcher, 0, sizeof(*launcher));
    }
}

static UmiStatus copy_text(char *destination,
                           size_t capacity,
                           const char *source)
{
    if (destination == NULL || source == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    const size_t length = strlen(source);
    if (length >= capacity) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    (void)memcpy(destination, source, length + 1U);
    return UMI_STATUS_OK;
}

UmiStatus umi_os_launcher_add(UmiOsLauncher *launcher,
                              const char *id,
                              const char *label,
                              const char *executable)
{
    if (launcher == NULL || id == NULL || label == NULL || executable == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    if (launcher->count >= UMI_OS_LAUNCHER_CAPACITY) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    UmiOsLaunchEntry *entry = &launcher->entries[launcher->count];
    UmiStatus status = copy_text(entry->id, sizeof(entry->id), id);
    if (status != UMI_STATUS_OK) {
        return status;
    }
    status = copy_text(entry->label, sizeof(entry->label), label);
    if (status != UMI_STATUS_OK) {
        return status;
    }
    status = copy_text(entry->executable, sizeof(entry->executable), executable);
    if (status != UMI_STATUS_OK) {
        return status;
    }
    ++launcher->count;
    return UMI_STATUS_OK;
}
