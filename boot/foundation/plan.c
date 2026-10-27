/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/plan.c
 *
 * PURPOSE:
 *   Inspect the real boot configuration and dependency order without starting services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "boot.h"
#include "boot_diagnostic.h"
#include <stdio.h>
#include <string.h>

/* The text format remains the default public interface. Preserve its original
 * entry point for review; optional JSON exposes the same validated plan to
 * native tools without granting a process-launch or filesystem-write action. */
#if 0
static int Usage(void)
{
    fputs("Usage: umicom-os-boot-plan CONFIG [--cmdline TEXT]\n"
          "Validate and display a boot plan without starting services.\n", stderr);
    return 2;
}

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 4) return Usage();
    if (argc == 4 && strcmp(argv[2], "--cmdline")) return Usage();

    /* The inspector shares the actual PID1 parser. It owns no mount, process,
     * credential or boot operation, so an ordinary account can review a plan. */
    FILE *input = fopen(argv[1], "rb");
    if (!input) {
        fputs("Cannot open the configuration file.\n", stderr);
        return 1;
    }
    char bytes[UMI_OS_CONFIG_MAX + 1U];
    size_t size = fread(bytes, 1, sizeof bytes, input);
    int failed = ferror(input);
    if (fclose(input)) failed = 1;
    if (failed || size > UMI_OS_CONFIG_MAX) {
        fputs("Cannot read a bounded configuration file.\n", stderr);
        return 1;
    }

    UmiOsBootConfig config;
    UmiOsBootDiagnostic diagnostic;
    if (!UmiOsBootConfigParseDetailed(bytes, size, &config, &diagnostic)) {
        /* Keep the earlier generic message for review; detailed diagnostics
         * now identify a line when the failure belongs to one declaration. */
#if 0
        fputs("Invalid boot configuration: check the header, fields, service paths and dependencies.\n", stderr);
#endif
        if (diagnostic.line) fprintf(stderr, "Line %zu: ", diagnostic.line);
        fprintf(stderr, "%s.\n", UmiOsBootProblemText(diagnostic.problem));
        return 1;
    }
    const char *commandLine = argc == 4 ? argv[3] : "";
    UmiOsBootOptions options;
    if (!UmiOsBootOptionsParse(commandLine, strlen(commandLine), &options)) {
        fputs("Invalid Umicom command-line controls.\n", stderr);
        return 1;
    }

    printf("UMICOM_OS_PLAN 1\nhostname=%s\nmode=%s\nconfigured=%zu\nscheduled=%zu\n",
        config.hostname, options.recovery ? "recovery" : "normal",
        config.count, options.recovery ? (size_t)0 : config.count);
    if (!options.recovery) {
        for (size_t i = 0; i < config.count; ++i) {
            const UmiOsService *service = &config.services[config.order[i]];
            printf("service=%s|%s|%s|%u\n", service->id, service->executable,
                service->dependency[0] ? service->dependency : "-",
                (unsigned)service->timeoutMs);
        }
    }
    puts("executed=false");
    return ferror(stdout) || fflush(stdout) ? 1 : 0;
}

#endif

static int Usage(void)
{
    fputs("Usage: umicom-os-boot-plan CONFIG [--cmdline TEXT] [--json]\n"
          "Validate and display a boot plan without starting services.\n", stderr);
    return 2;
}

/* Configuration has already passed the production parser. Escape at the output
 * boundary anyway so future grammar extensions cannot silently break JSON. */
static void JsonString(const char *text)
{
    putchar('"');
    for (const unsigned char *at = (const unsigned char *)text; *at; ++at) {
        if (*at == '"' || *at == '\\') { putchar('\\'); putchar(*at); }
        else if (*at < 0x20U) printf("\\u%04x", (unsigned)*at);
        else putchar(*at);
    }
    putchar('"');
}

static void JsonPlan(const UmiOsBootConfig *config, const UmiOsBootOptions *options)
{
    fputs("{\"schema\":\"umicom.os.boot-plan\",\"version\":1,\"hostname\":", stdout);
    JsonString(config->hostname);
    printf(",\"mode\":\"%s\",\"configured\":%zu,\"scheduled\":%zu,"
           "\"autopoweroff\":%s,\"executed\":false,\"services\":[",
        options->recovery ? "recovery" : "normal", config->count,
        options->recovery ? (size_t)0 : config->count,
        options->autopoweroff ? "true" : "false");
    if (!options->recovery) {
        for (size_t i = 0; i < config->count; ++i) {
            const UmiOsService *service = &config->services[config->order[i]];
            if (i) putchar(',');
            fputs("{\"id\":", stdout); JsonString(service->id);
            fputs(",\"executable\":", stdout); JsonString(service->executable);
            fputs(",\"dependency\":", stdout);
            if (service->dependency[0]) JsonString(service->dependency);
            else fputs("null", stdout);
            printf(",\"timeoutMs\":%u}", (unsigned)service->timeoutMs);
        }
    }
    puts("]}");
}

int main(int argc, char **argv)
{
    if (argc < 2) return Usage();
    bool json = false, hasCommandLine = false;
    const char *commandLine = "";
    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "--json") && !json) json = true;
        else if (!strcmp(argv[i], "--cmdline") && !hasCommandLine && i + 1 < argc) {
            if (!strcmp(argv[i + 1], "--json") || !strcmp(argv[i + 1], "--cmdline"))
                return Usage();
            commandLine = argv[++i];
            hasCommandLine = true;
        } else return Usage();
    }

    /* Reuse the PID1 parser without importing PID1's privileged operations. */
    FILE *input = fopen(argv[1], "rb");
    if (!input) {
        fputs("Cannot open the configuration file.\n", stderr);
        return 1;
    }
    char bytes[UMI_OS_CONFIG_MAX + 1U];
    size_t size = fread(bytes, 1, sizeof bytes, input);
    int failed = ferror(input);
    if (fclose(input)) failed = 1;
    if (failed || size > UMI_OS_CONFIG_MAX) {
        fputs("Cannot read a bounded configuration file.\n", stderr);
        return 1;
    }
    UmiOsBootConfig config;
    UmiOsBootDiagnostic diagnostic;
    if (!UmiOsBootConfigParseDetailed(bytes, size, &config, &diagnostic)) {
        if (diagnostic.line) fprintf(stderr, "Line %zu: ", diagnostic.line);
        fprintf(stderr, "%s.\n", UmiOsBootProblemText(diagnostic.problem));
        return 1;
    }
    UmiOsBootOptions options;
    if (!UmiOsBootOptionsParse(commandLine, strlen(commandLine), &options)) {
        fputs("Invalid Umicom command-line controls.\n", stderr);
        return 1;
    }
    /* Validate every input before the first output byte in either format. */
    if (json) JsonPlan(&config, &options);
    else {
        printf("UMICOM_OS_PLAN 1\nhostname=%s\nmode=%s\nconfigured=%zu\nscheduled=%zu\n",
            config.hostname, options.recovery ? "recovery" : "normal",
            config.count, options.recovery ? (size_t)0 : config.count);
        if (!options.recovery) {
            for (size_t i = 0; i < config.count; ++i) {
                const UmiOsService *service = &config.services[config.order[i]];
                printf("service=%s|%s|%s|%u\n", service->id, service->executable,
                    service->dependency[0] ? service->dependency : "-",
                    (unsigned)service->timeoutMs);
            }
        }
        puts("executed=false");
    }
    return ferror(stdout) || fflush(stdout) ? 1 : 0;
}
