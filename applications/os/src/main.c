/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/main.c
 *
 * PURPOSE:
 *   Run the existing user-space startup path or inspect a captured guest boot report.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* BEGINNER NOTE:
 * This file keeps one responsibility small and explicit. Read the public
 * structure/function declarations first, then follow the implementation in
 * the matching source file.
 */
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "umicom/platform/boot_report_file.h"
#include "umicom/platform/native_arguments.h"

#include "umicom/os/application.h"
#include "umicom/os/services.h"
#include "umicom/os/version.h"

/* Retain the original entry point for review. RunStartup keeps its lifecycle
 * unchanged; explicit inspection commands use Framework without starting it. */
#if 0
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
#endif

static int RunStartup(void)
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

static int Usage(void)
{
    fputs("Usage: umicom-os [--help]\n"
          "       umicom-os --boot-report ABSOLUTE_FILE [--source SHA256]\n"
          "Inspect a captured report. No guest, service or privileged operation is started.\n",
          stdout);
    return ferror(stdout) || fflush(stdout) ? 1 : 0;
}
static int InspectReport(const char *path, const char *source)
{
    UmiBootReport report;
    UmiStatus status = UmiBootReportLoad(path, source, &report);
    if (status != UMI_STATUS_OK) {
        fprintf(stderr, "Cannot inspect the selected boot report (status=%d). "
            "Check the absolute regular-file path, report format and expected source identity.\n",
            (int)status);
        return 1;
    }
    printf("Captured boot report\nmode=%s\nstate=%s\ncompleted=%" PRIu32
        "/%" PRIu32 "\nreason=%s\nsource=%s\n",
        report.mode == UMI_BOOT_REPORT_NORMAL ? "normal" : "recovery",
        UmiBootReportStateText(report.state), report.completed, report.planned,
        report.reason, report.sourceId);
    puts("This file is an observation, not proof of current machine health or publisher identity.");
    return ferror(stdout) || fflush(stdout) ? 1 : 0;
}
static int CommandMain(int argc, char **argv)
{
    if (argc == 1) return RunStartup();
    if (argc == 2 && !strcmp(argv[1], "--help")) return Usage();
    if ((argc == 3 || argc == 5) && !strcmp(argv[1], "--boot-report")) {
        if (argc == 5 && strcmp(argv[3], "--source")) return 2;
        return InspectReport(argv[2], argc == 5 ? argv[4] : NULL);
    }
    (void)Usage();
    return 2;
}

/* Capture Windows Unicode arguments through Framework instead of interpreting
 * native paths in the current ANSI code page. POSIX argument bytes are retained. */
int main(int argc, char **argv)
{
    int result = 1;
    if (UmiNativeArgumentsDispatch(argc, argv, CommandMain, &result) != UMI_STATUS_OK) {
        fputs("Cannot capture native command arguments.\n", stderr);
        return 1;
    }
    return result;
}
