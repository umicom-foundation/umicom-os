/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/main.c
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
#include <stdio.h>

#include "umicom/os/application.h"
#include "umicom/os/services.h"
#include "umicom/os/version.h"

int main(void)
{
    UmiOsApplication application;
    umi_os_application_init(&application);
    if (umi_os_application_start(&application) != UMI_STATUS_OK) {
        return 1;
    }

    (void)printf("Umicom OS user-space %s\n", UMI_OS_USERSPACE_VERSION);
    (void)printf("Framework services: %zu\n",
                 umi_os_required_framework_service_count());

    (void)umi_os_application_stop(&application);
    return 0;
}
