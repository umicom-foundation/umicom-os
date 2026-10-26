/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/guest_environment.h
 *
 * PURPOSE:
 *   Describe the observed restrictions of a packaged guest service independently of Linux system calls.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_OS_GUEST_ENVIRONMENT_H
#define UMICOM_OS_GUEST_ENVIRONMENT_H
#include <stdbool.h>
#include <stdint.h>

enum {
    UMI_OS_GUEST_IDENTITY = 1U << 0,
    UMI_OS_GUEST_GROUPS = 1U << 1,
    UMI_OS_GUEST_PRIVILEGES = 1U << 2,
    UMI_OS_GUEST_LIMITS = 1U << 3,
    UMI_OS_GUEST_WORKSPACE = 1U << 4,
    UMI_OS_GUEST_PROC = 1U << 5,
    UMI_OS_GUEST_SYS = 1U << 6,
    UMI_OS_GUEST_RUN = 1U << 7,
    UMI_OS_GUEST_TMP = 1U << 8,
    UMI_OS_GUEST_ALL = (1U << 9) - 1U
};
typedef struct UmiOsGuestMount {
    bool expectedType;
    bool restricted;
} UmiOsGuestMount;
typedef struct UmiOsGuestEnvironment {
    /* A failed observation is not a zero-valued successful observation.
     * Producers set a bit only after all calls for that section succeed. */
    uint32_t observed;
    uint64_t uid, effectiveUid, gid, effectiveGid;
    int supplementaryGroups;
    int noNewPrivileges;
    uint64_t coreLimit, fileLimit, descriptorLimit;
    uint64_t coreHardLimit, fileHardLimit, descriptorHardLimit;
    uint64_t workUid, workGid;
    uint32_t workMode;
    bool workDirectory, workingDirectoryMatches;
    UmiOsGuestMount proc, sys, run, tmp;
} UmiOsGuestEnvironment;
typedef enum UmiOsGuestProblem {
    UMI_OS_GUEST_OK = 0,
    UMI_OS_GUEST_ARGUMENT,
    UMI_OS_GUEST_OBSERVATION,
    UMI_OS_GUEST_WRONG_IDENTITY,
    UMI_OS_GUEST_EXTRA_GROUPS,
    UMI_OS_GUEST_PRIVILEGE_GAIN,
    UMI_OS_GUEST_UNBOUNDED_LIMITS,
    UMI_OS_GUEST_UNSAFE_WORKSPACE,
    UMI_OS_GUEST_UNSAFE_PROC,
    UMI_OS_GUEST_UNSAFE_SYS,
    UMI_OS_GUEST_UNSAFE_RUN,
    UMI_OS_GUEST_UNSAFE_TMP
} UmiOsGuestProblem;

/* This is a policy check of an observation, not evidence that the supplied
 * values came from a real guest. The Linux platform probe supplies those
 * observations; host tests intentionally use synthetic values. */
UmiOsGuestProblem UmiOsGuestEnvironmentCheck(const UmiOsGuestEnvironment *environment);
const char *UmiOsGuestProblemText(UmiOsGuestProblem problem);
#endif
