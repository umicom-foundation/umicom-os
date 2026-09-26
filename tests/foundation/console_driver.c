/* Umicom OS console tests. Sammy Hegab, Umicom Foundation. Licence: MIT.
 * This ordinary process exercises console input only; it is not init and does
 * not mount filesystems, launch services or request an operating-system reboot. */
#include "boot.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
volatile sig_atomic_t UmiOsStopRequested=0;
int main(void)
{
    UmiOsBootConfig config={0};
    config.count=1;strcpy(config.services[0].id,"platform-check");
    UmiOsRecoveryConsole(&config);
    printf("STOP_REQUEST=%d\n",(int)UmiOsStopRequested);
    return 0;
}
