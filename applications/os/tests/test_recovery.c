/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/tests/test_recovery.c
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
#include "umicom/os/recovery.h"
int main(void){UmiOsRecoveryBoundary b=umi_os_recovery_boundary();return !b.framework_required?0:1;}
