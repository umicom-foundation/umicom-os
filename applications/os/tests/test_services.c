/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/tests/test_services.c
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
#include "umicom/os/services.h"
int main(void){return umi_os_required_framework_service_count()>=6U?0:1;}
