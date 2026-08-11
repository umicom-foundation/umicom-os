/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/services.c
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
#include "umicom/os/services.h"
static const char *SERVICES[]={"umicom.runtime","umicom.platform","umicom.security","umicom.messaging","umicom.ui","umicom.ui.components"};
size_t umi_os_required_framework_service_count(void){return sizeof(SERVICES)/sizeof(SERVICES[0]);}const char *umi_os_required_framework_service(size_t i){return i<umi_os_required_framework_service_count()?SERVICES[i]:NULL;}
