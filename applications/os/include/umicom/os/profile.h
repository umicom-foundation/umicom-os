/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/profile.h
 *
 * PURPOSE:
 *   Represent developer, desktop, server, RISC-V and recovery OS profiles.
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
#ifndef UMICOM_OS_PROFILE_H
#define UMICOM_OS_PROFILE_H
typedef enum UmiOsProfile { UMI_OS_PROFILE_DEVELOPER=1,UMI_OS_PROFILE_DESKTOP=2,UMI_OS_PROFILE_SERVER=3,UMI_OS_PROFILE_RECOVERY=4,UMI_OS_PROFILE_RISCV64=5,UMI_OS_PROFILE_CHERI_RISCV64=6 } UmiOsProfile;
const char *umi_os_profile_name(UmiOsProfile profile);
#endif
