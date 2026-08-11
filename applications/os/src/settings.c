/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/settings.c
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
#include "umicom/os/settings.h"
UmiOsSettings umi_os_settings_default(void){UmiOsSettings s={false,true,false};return s;}
