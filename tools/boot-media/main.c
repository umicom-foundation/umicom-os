/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tools/boot-media/main.c
 *
 * PURPOSE:
 *   Route the installed SDK console through shared native argument ownership and boot-media commands.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include "umicom/boot_media/media.h"
#include "umicom/platform/native_arguments.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    int result = 1;
    /* The application owns only this process boundary. Framework captures the
     * original Windows Unicode command line and owns every media operation. */
    UmiStatus status = UmiNativeArgumentsDispatch(argc, argv, UmiBootMediaMain, &result);
    if (status != UMI_STATUS_OK) {
        fprintf(stderr, "Umicom OS Boot Media: argument capture failed (%d)\n", (int)status);
        return 1;
    }
    /* A command's successful return must not hide undelivered console output. */
    if (fflush(stdout) != 0 || ferror(stdout) || fflush(stderr) != 0 || ferror(stderr))
        return 1;
    return result;
}
