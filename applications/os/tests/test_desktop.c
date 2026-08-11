/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/tests/test_desktop.c
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
#include "umicom/os/desktop.h"
int main(void){UmiOsDesktop d;umi_os_desktop_init(&d);return d.system_panel_count>=1U?0:1;}
