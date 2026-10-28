/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/cvm-probe/main.c
 *
 * PURPOSE:
 *   Reuse Umicom C through a bounded, optional user-space consumer.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom_c/umicom_c.h"
#include <stdio.h>

/* This Linux user-space probe uses the same runtime as the freestanding Kernel
 * program. It accepts no paths or commands and performs no privileged work. */
int main(void)
{
    static const UmcInstruction program[] = {
        {UMC_PUSH, 7, 0U}, {UMC_PUSH, 6, 0U},
        {UMC_MULTIPLY, 0, 0U}, {UMC_RETURN, 0, 0U}
    };
    UmcResult result;
    UmcDiagnostic diagnostic;
    const UmcStatus status = umc_execute(program, 4U, 4U, &result, &diagnostic);
    if (status != UMC_OK || result.value != 42 || result.instructions_executed != 4U) {
        fputs("UMICOM_CVM_PROBE_FAILED\n", stderr);
        return 1;
    }
    puts("UMICOM_CVM_PROBE_READY value=42");
    return 0;
}
