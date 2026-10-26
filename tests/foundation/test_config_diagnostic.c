/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tests/foundation/test_config_diagnostic.c
 *
 * PURPOSE:
 *   Verify rejection reasons, line positions and unchanged accepted state after invalid input.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "boot_diagnostic.h"
#include <stdio.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr, "check %d: %s\n", __LINE__, #e); return 1; } } while (0)
static const char GOOD[] = "UMICOM_OS_BOOT 1\nhostname=lesson\n"
    "service=one|/usr/libexec/umicom-one|-|1000\n";
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    char text[UMI_OS_CONFIG_MAX] = {0};
    strcpy(text, GOOD);
    UmiOsBootProblem expected = UMI_OS_BOOT_PROBLEM_NONE;
    size_t line = 0;
    if (!strcmp(argv[1], "service")) {
        strcpy(text, "UMICOM_OS_BOOT 1\nhostname=lesson\nservice=one|/bin/sh|-|1000\n");
        expected = UMI_OS_BOOT_PROBLEM_SERVICE; line = 3;
    } else if (!strcmp(argv[1], "hostname")) {
        strcat(text, "hostname=other\n");
        expected = UMI_OS_BOOT_PROBLEM_HOSTNAME; line = 4;
    } else if (!strcmp(argv[1], "character")) {
        text[sizeof "UMICOM_OS_BOOT 1\nhostname=" - 1U] = '\t';
        expected = UMI_OS_BOOT_PROBLEM_CHARACTER; line = 2;
    } else if (!strcmp(argv[1], "field")) {
        strcat(text, "unknown=value\n");
        expected = UMI_OS_BOOT_PROBLEM_FIELD; line = 4;
    } else if (!strcmp(argv[1], "graph")) {
        strcpy(text, "UMICOM_OS_BOOT 1\nhostname=lesson\nservice=one|/usr/libexec/umicom-one|missing|1000\n");
        expected = UMI_OS_BOOT_PROBLEM_GRAPH;
    } else if (!strcmp(argv[1], "format")) {
        text[strlen(text) - 1U] = 0;
        expected = UMI_OS_BOOT_PROBLEM_FORMAT;
    } else if (!strcmp(argv[1], "required")) {
        strcpy(text, "UMICOM_OS_BOOT 1\nhostname=lesson\n");
        expected = UMI_OS_BOOT_PROBLEM_REQUIRED;
    } else if (!strcmp(argv[1], "capacity")) {
        strcpy(text, "UMICOM_OS_BOOT 1\nhostname=lesson\n");
        for (unsigned i = 0; i <= UMI_OS_SERVICE_MAX; ++i) {
            char record[128];
            int n = snprintf(record, sizeof record,
                "service=item%u|/usr/libexec/umicom-check|-|1000\n", i);
            CHECK(n > 0 && (size_t)n < sizeof record);
            CHECK(strlen(text) + (size_t)n < sizeof text);
            strcat(text, record);
        }
        expected = UMI_OS_BOOT_PROBLEM_CAPACITY; line = 19;
    } else if (strcmp(argv[1], "valid") && strcmp(argv[1], "argument")) return 2;

    UmiOsBootConfig value;
    CHECK(UmiOsBootConfigParse(GOOD, sizeof GOOD - 1U, &value));
    UmiOsBootConfig saved = value;
    UmiOsBootDiagnostic diagnostic = {UMI_OS_BOOT_PROBLEM_GRAPH, 99};
    if (!strcmp(argv[1], "argument")) {
        CHECK(!UmiOsBootConfigParseDetailed(NULL, 0, &value, &diagnostic));
        CHECK(diagnostic.problem == UMI_OS_BOOT_PROBLEM_ARGUMENT && !diagnostic.line);
        CHECK(!memcmp(&value, &saved, sizeof value));
        CHECK(!UmiOsBootConfigParseDetailed(GOOD, sizeof GOOD - 1U, NULL, &diagnostic));
        CHECK(UmiOsBootConfigParseDetailed(GOOD, sizeof GOOD - 1U, &value, NULL));
        return 0;
    }
    bool accepted = UmiOsBootConfigParseDetailed(text, strlen(text), &value, &diagnostic);
    CHECK(accepted == (expected == UMI_OS_BOOT_PROBLEM_NONE));
    CHECK(diagnostic.problem == expected && diagnostic.line == line);
    CHECK(UmiOsBootProblemText(diagnostic.problem)[0] != '\0');
    if (!accepted) CHECK(!memcmp(&value, &saved, sizeof value));
    UmiOsBootConfig compatibility = saved;
    CHECK(UmiOsBootConfigParse(text, strlen(text), &compatibility) == accepted);
    CHECK(!memcmp(&compatibility, &value, sizeof value));
    return 0;
}
