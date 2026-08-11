/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/tests/test_settings.c
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
#include "umicom/os/settings.h"
int main(void){UmiOsSettings s=umi_os_settings_default();return s.animations?0:1;}
