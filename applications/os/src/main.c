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
#if defined(UMICOM_OS_HAS_BOOT_REPORT_JSON)
#include "umicom/platform/boot_report_json.h"
#endif

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
          "       umicom-os --boot-report-identity ABSOLUTE_FILE [--source SHA256] [--capture SHA256]\n"
          "       umicom-os --compare-boot-reports EARLIER_ABSOLUTE_FILE LATER_ABSOLUTE_FILE\n"
          "       umicom-os --boot-report-json ABSOLUTE_FILE [--source SHA256]\n"
          "       umicom-os --compare-boot-reports-json EARLIER_ABSOLUTE_FILE LATER_ABSOLUTE_FILE\n"
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

static int InspectIdentity(int argc, char **argv)
{
    const char *source = NULL, *capture = NULL;
    /* Admission is independent of the selected Framework. Refuse incomplete,
     * repeated or unknown flags before reading the selected local file. */
    if (argc < 3 || argc > 7 || argc % 2 == 0) return 2;
    for (int index = 3; index < argc; index += 2) {
        if (!strcmp(argv[index], "--source") && source == NULL) source = argv[index + 1];
        else if (!strcmp(argv[index], "--capture") && capture == NULL) capture = argv[index + 1];
        else return 2;
    }
#if defined(UMI_BOOT_REPORT_HAS_CAPTURE_IDENTITY)
    /* Framework parses, checks and hashes one native read. This branch only
     * presents its result and never enters the application startup route. */
    UmiBootReportCapture captured;
    UmiStatus status = UmiBootReportInspectFile(argv[2], source, capture, &captured);
    if (status != UMI_STATUS_OK) {
        fprintf(stderr, "Cannot confirm captured boot report identity (status=%d). "
            "Check the selected file and both expected identities.\n", (int)status);
        return 1;
    }
    printf("Captured boot report identity\nsource=%s\ncapture=%s\nbytes=%zu\nstate=%s\n",
        captured.report.sourceId, captured.captureSha256, captured.byteCount,
        UmiBootReportStateText(captured.report.state));
    puts("This file is an observation, not proof of current machine health or publisher identity.");
    return ferror(stdout) || fflush(stdout) ? 1 : 0;
#else
    /* Allow older dependency checkouts to keep their established read and
     * startup commands until the primary Framework is published and aligned. */
    fputs("The selected Framework lacks capture identity.\n", stderr);
    return 1;
#endif
}

/* Keep admission and comparison in Framework. This frontend formats observed
 * fields without starting a guest or interpreting a difference as a repair. */
static int CompareReports(const char *beforePath, const char *afterPath)
{
    UmiBootReport before, after;
    UmiStatus status = UmiBootReportLoad(beforePath, NULL, &before);
    if (status != UMI_STATUS_OK) {
        fprintf(stderr, "Cannot read earlier report (status=%d).\n", (int)status);
        return 1;
    }
    status = UmiBootReportLoad(afterPath, NULL, &after);
    if (status != UMI_STATUS_OK) {
        fprintf(stderr, "Cannot read later report (status=%d).\n", (int)status);
        return 1;
    }
    uint32_t changes = 0U;
    status = UmiBootReportCompare(&before, &after, &changes);
    if (status != UMI_STATUS_OK) {
        fprintf(stderr, "Cannot compare reports (status=%d).\n", (int)status);
        return 1;
    }
    puts("Captured boot report comparison");
    if (changes == 0U) puts("No semantic fields changed.");
    if (changes & UMI_BOOT_REPORT_CHANGED_MODE)
        printf("mode: %s -> %s\n", before.mode == UMI_BOOT_REPORT_NORMAL ? "normal" : "recovery",
            after.mode == UMI_BOOT_REPORT_NORMAL ? "normal" : "recovery");
    if (changes & UMI_BOOT_REPORT_CHANGED_STATE)
        printf("state: %s -> %s\n", UmiBootReportStateText(before.state), UmiBootReportStateText(after.state));
    if (changes & UMI_BOOT_REPORT_CHANGED_PLANNED)
        printf("planned: %" PRIu32 " -> %" PRIu32 "\n", before.planned, after.planned);
    if (changes & UMI_BOOT_REPORT_CHANGED_COMPLETED)
        printf("completed: %" PRIu32 " -> %" PRIu32 "\n", before.completed, after.completed);
    if (changes & UMI_BOOT_REPORT_CHANGED_REASON)
        printf("reason: %s -> %s\n", before.reason, after.reason);
    if (changes & UMI_BOOT_REPORT_CHANGED_SOURCE) {
        printf("source: %s -> %s\n", before.sourceId, after.sourceId);
        puts("Source identities differ; review the boot plans before interpreting progress.");
    }
    puts("Reports are observations, not proof of current machine health or publisher identity.");
    return ferror(stdout) || fflush(stdout) ? 1 : 0;
}


/* Framework owns schema and validation. The console owns only argument
 * routing and stdout. Older Framework selections retain existing text modes
 * and refuse JSON explicitly rather than running startup or copying a parser. */
static int InspectJson(const char *beforePath, const char *afterPath, const char *source)
{
#if defined(UMICOM_OS_HAS_BOOT_REPORT_JSON)
    UmiBootReport before, after;
    UmiStatus status = UmiBootReportLoad(beforePath, source, &before);
    if (status != UMI_STATUS_OK) {
        fprintf(stderr, "Cannot read selected boot report for JSON (status=%d).\n", (int)status);
        return 1;
    }
    if (afterPath) {
        status = UmiBootReportLoad(afterPath, NULL, &after);
        if (status != UMI_STATUS_OK) {
            fprintf(stderr, "Cannot read later boot report for JSON (status=%d).\n", (int)status);
            return 1;
        }
    }
    char json[UMI_BOOT_REPORT_JSON_CAPACITY]; size_t length = 0U;
    status = afterPath ? UmiBootReportComparisonJson(&before, &after, json, sizeof json, &length) :
        UmiBootReportJson(&before, json, sizeof json, &length);
    if (status != UMI_STATUS_OK) {
        fprintf(stderr, "Cannot format captured boot JSON (status=%d).\n", (int)status);
        return 1;
    }
    if (fwrite(json, 1U, length, stdout) != length || fputc('\n', stdout) == EOF) return 1;
    return fflush(stdout) == 0 ? 0 : 1;
#else
    (void)beforePath; (void)afterPath; (void)source;
    fputs("Boot-report JSON is unavailable in the selected Framework. Select an SDK with boot_report_tools.\n", stderr);
    return 1;
#endif
}

static int CommandMain(int argc, char **argv)
{
    /* Exact capture inspection is always explicit, including missing-file usage. */
    if (argc >= 2 && !strcmp(argv[1], "--boot-report-identity")) return InspectIdentity(argc, argv);
    if (argc == 4 && !strcmp(argv[1], "--compare-boot-reports-json"))
        return InspectJson(argv[2], argv[3], NULL);
    if ((argc == 3 || argc == 5) && !strcmp(argv[1], "--boot-report-json")) {
        if (argc == 5 && strcmp(argv[3], "--source")) return 2;
        return InspectJson(argv[2], NULL, argc == 5 ? argv[4] : NULL);
    }
    if (argc == 4 && !strcmp(argv[1], "--compare-boot-reports"))
        return CompareReports(argv[2], argv[3]);
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
