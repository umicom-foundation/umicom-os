/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/guest_environment.c
 *
 * PURPOSE:
 *   Evaluate the minimal guest service restrictions without mounting filesystems or changing credentials.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "guest_environment.h"
#include "boot.h"

UmiOsGuestProblem UmiOsGuestEnvironmentCheck(const UmiOsGuestEnvironment *e)
{
    if (!e) return UMI_OS_GUEST_ARGUMENT;
    if (e->observed != UMI_OS_GUEST_ALL) return UMI_OS_GUEST_OBSERVATION;
    if (e->uid != 1000 || e->effectiveUid != 1000 ||
        e->gid != 1000 || e->effectiveGid != 1000)
        return UMI_OS_GUEST_WRONG_IDENTITY;
    if (e->supplementaryGroups != 0) return UMI_OS_GUEST_EXTRA_GROUPS;
    if (e->noNewPrivileges != 1) return UMI_OS_GUEST_PRIVILEGE_GAIN;
    if (e->coreLimit != 0 || e->fileLimit == 0 || e->fileLimit > UMI_OS_LOG_MAX ||
        e->descriptorLimit < 3 || e->descriptorLimit > 32 ||
        e->coreHardLimit != 0 || e->fileHardLimit < e->fileLimit ||
        e->fileHardLimit > UMI_OS_LOG_MAX || e->descriptorHardLimit < e->descriptorLimit ||
        e->descriptorHardLimit > 32)
        return UMI_OS_GUEST_UNBOUNDED_LIMITS;
    if (!e->workDirectory || !e->workingDirectoryMatches ||
        e->workUid != 1000 || e->workGid != 1000 || e->workMode != 0700U)
        return UMI_OS_GUEST_UNSAFE_WORKSPACE;
    if (!e->proc.expectedType || !e->proc.restricted) return UMI_OS_GUEST_UNSAFE_PROC;
    if (!e->sys.expectedType || !e->sys.restricted) return UMI_OS_GUEST_UNSAFE_SYS;
    if (!e->run.expectedType || !e->run.restricted) return UMI_OS_GUEST_UNSAFE_RUN;
    if (!e->tmp.expectedType || !e->tmp.restricted) return UMI_OS_GUEST_UNSAFE_TMP;
    return UMI_OS_GUEST_OK;
}
const char *UmiOsGuestProblemText(UmiOsGuestProblem problem)
{
    switch (problem) {
    case UMI_OS_GUEST_OK: return "service-environment-valid";
    case UMI_OS_GUEST_ARGUMENT: return "missing-environment";
    case UMI_OS_GUEST_OBSERVATION: return "incomplete-environment-observation";
    case UMI_OS_GUEST_WRONG_IDENTITY: return "unexpected-service-identity";
    case UMI_OS_GUEST_EXTRA_GROUPS: return "supplementary-groups-present";
    case UMI_OS_GUEST_PRIVILEGE_GAIN: return "privilege-gain-not-disabled";
    case UMI_OS_GUEST_UNBOUNDED_LIMITS: return "service-resource-limits-invalid";
    case UMI_OS_GUEST_UNSAFE_WORKSPACE: return "service-workspace-invalid";
    case UMI_OS_GUEST_UNSAFE_PROC: return "proc-mount-invalid";
    case UMI_OS_GUEST_UNSAFE_SYS: return "sys-mount-invalid";
    case UMI_OS_GUEST_UNSAFE_RUN: return "run-mount-invalid";
    case UMI_OS_GUEST_UNSAFE_TMP: return "tmp-mount-invalid";
    default: return "unknown-environment-problem";
    }
}
