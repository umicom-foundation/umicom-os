/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/tests/test_launcher.c
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
#include "umicom/os/launcher.h"
int main(void){UmiOsLauncher l;umi_os_launcher_init(&l);return umi_os_launcher_add(&l,"studio","Studio IDE","umicom-studio-ide")==UMI_STATUS_OK&&l.count==1U?0:1;}
