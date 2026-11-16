/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tools/web-preview/main.c
 *
 * PURPOSE:
 *   Compose the shared Framework preview command through native Unicode argument ownership, without duplicating server behavior.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/platform/native_arguments.h"
#include "umicom/web/preview_command.h"
#include <stdio.h>
static int PreviewMain(int argc, char **argv)
{
    if (argc < 1 || argv == NULL) return 2;
    return UmiWebPreviewCommand(argc - 1, argv + 1);
}
int main(int argc, char **argv)
{
    int result = 1;
    UmiStatus status = UmiNativeArgumentsDispatch(argc, argv, PreviewMain, &result);
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr, "Umicom OS Web Preview: argument capture failed (%d)\n", (int)status);
        return 1;
    }
    if (fflush(stdout) != 0 || ferror(stdout) || fflush(stderr) != 0 || ferror(stderr)) return 1;
    return result;
}
