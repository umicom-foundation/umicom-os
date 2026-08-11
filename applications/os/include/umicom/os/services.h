/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/services.h
 *
 * PURPOSE:
 *   Describe which shared Framework services the normal OS desktop expects.
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
#ifndef UMICOM_OS_SERVICES_H
#define UMICOM_OS_SERVICES_H
#include <stddef.h>
size_t umi_os_required_framework_service_count(void);
const char *umi_os_required_framework_service(size_t index);
#endif
