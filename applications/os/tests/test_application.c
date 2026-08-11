/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/tests/test_application.c
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
#include "umicom/os/application.h"
int main(void){UmiOsApplication a;umi_os_application_init(&a);if(umi_os_application_start(&a)!=UMI_STATUS_OK)return 1;if(!a.running)return 2;return umi_os_application_stop(&a)==UMI_STATUS_OK?0:3;}
