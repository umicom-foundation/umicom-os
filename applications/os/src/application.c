/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/application.c
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
#include "umicom/os/application.h"
void umi_os_application_init(UmiOsApplication*a){if(a){a->initialised=1;a->running=0;}}
UmiStatus umi_os_application_start(UmiOsApplication*a){if(!a||!a->initialised)return UMI_STATUS_INVALID_STATE;a->running=1;return UMI_STATUS_OK;}
UmiStatus umi_os_application_stop(UmiOsApplication*a){if(!a||!a->initialised)return UMI_STATUS_INVALID_STATE;a->running=0;return UMI_STATUS_OK;}
