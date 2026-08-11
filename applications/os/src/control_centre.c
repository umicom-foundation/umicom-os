/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/control_centre.c
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
#include "umicom/os/control_centre.h"
void umi_os_control_centre_init(UmiOsControlCentre*c){if(c)c->panel_count=4U;}
