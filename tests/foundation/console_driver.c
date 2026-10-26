/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tests/foundation/console_driver.c
 *
 * PURPOSE:
 *   Exercise console input as an ordinary process without guest privileges.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* This ordinary process exercises console input only; it is not init and does
 * not mount filesystems, launch services or request an operating-system reboot. */
#include "boot.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
volatile sig_atomic_t UmiOsStopRequested=0;
int main(int argc, char **argv)
{
    UmiOsBootConfig config={0};
    /* Keep the original hand-written fixture for review. The production
     * parser now supplies a forward dependency and complete service fields. */
#if 0
    config.count=1;strcpy(config.services[0].id,"platform-check");
#endif

    /* The parser supplies the same bounded, ordered structure used by PID1.
     * --empty models unavailable configuration; neither route starts services. */
    if (argc == 2 && !strcmp(argv[1], "--empty")) {
        /* Leave the zero-initialized configuration unavailable. */
    } else if (argc == 1) {
        const char plan[] = "UMICOM_OS_BOOT 1\nhostname=console-test\n"
            "service=framework-probe|/usr/libexec/umicom-framework-probe|platform-check|5000\n"
            "service=platform-check|/usr/libexec/umicom-platform-check|-|1000\n";
        if (!UmiOsBootConfigParse(plan, sizeof plan - 1U, &config)) return 1;
    } else return 2;

    UmiOsRecoveryConsole(&config);
    printf("STOP_REQUEST=%d\n",(int)UmiOsStopRequested);
    return 0;
}
