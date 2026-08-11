/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/control_centre.h
 *
 * PURPOSE:
 *   Summarise system panels exposed by the Umicom OS control centre.
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
#ifndef UMICOM_OS_CONTROL_CENTRE_H
#define UMICOM_OS_CONTROL_CENTRE_H
#include <stddef.h>
typedef struct UmiOsControlCentre { size_t panel_count; } UmiOsControlCentre;
void umi_os_control_centre_init(UmiOsControlCentre *centre);
#endif
