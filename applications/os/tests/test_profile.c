/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/tests/test_profile.c
 *
 * PURPOSE:
 *   Test one first-stage Umicom OS user-space boundary or model.
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
#include <string.h>
#include "umicom/os/profile.h"
int main(void){return strcmp(umi_os_profile_name(UMI_OS_PROFILE_RISCV64),"riscv64")==0?0:1;}
