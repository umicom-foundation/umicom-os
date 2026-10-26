/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/boot_diagnostic.h
 *
 * PURPOSE:
 *   Explain configuration rejection without changing the accepted boot-plan grammar.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_OS_BOOT_DIAGNOSTIC_H
#define UMICOM_OS_BOOT_DIAGNOSTIC_H
#include "boot.h"
typedef enum UmiOsBootProblem {
    UMI_OS_BOOT_PROBLEM_NONE,
    UMI_OS_BOOT_PROBLEM_ARGUMENT,
    UMI_OS_BOOT_PROBLEM_FORMAT,
    UMI_OS_BOOT_PROBLEM_CHARACTER,
    UMI_OS_BOOT_PROBLEM_HOSTNAME,
    UMI_OS_BOOT_PROBLEM_SERVICE,
    UMI_OS_BOOT_PROBLEM_CAPACITY,
    UMI_OS_BOOT_PROBLEM_FIELD,
    UMI_OS_BOOT_PROBLEM_REQUIRED,
    UMI_OS_BOOT_PROBLEM_GRAPH
} UmiOsBootProblem;
typedef struct UmiOsBootDiagnostic {
    UmiOsBootProblem problem;
    size_t line; /* One-based, or zero when the problem describes the whole plan. */
} UmiOsBootDiagnostic;

/* Uses the same grammar and transactional output as UmiOsBootConfigParse.
 * diagnostic is optional and receives a reason on both success and failure.
 * Input, output and diagnostic storage must not overlap. No input is retained. */
bool UmiOsBootConfigParseDetailed(const char *data, size_t size,
    UmiOsBootConfig *out, UmiOsBootDiagnostic *diagnostic);
const char *UmiOsBootProblemText(UmiOsBootProblem problem);
#endif
