/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/profile.c
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
#include "umicom/os/profile.h"

const char *umi_os_profile_name(UmiOsProfile profile)
{
    switch (profile) {
        case UMI_OS_PROFILE_DEVELOPER: return "developer";
        case UMI_OS_PROFILE_DESKTOP: return "desktop";
        case UMI_OS_PROFILE_SERVER: return "server";
        case UMI_OS_PROFILE_RECOVERY: return "recovery";
        case UMI_OS_PROFILE_RISCV64: return "riscv64";
        case UMI_OS_PROFILE_CHERI_RISCV64: return "cheri-riscv64-experimental";
        default: return "unknown";
    }
}
