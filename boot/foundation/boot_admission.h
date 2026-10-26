/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/boot_admission.h
 *
 * PURPOSE:
 *   Describe startup admission without mounting filesystems or starting services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_OS_BOOT_ADMISSION_H
#define UMICOM_OS_BOOT_ADMISSION_H
#include "boot.h"

typedef struct UmiOsBootInputs {
    bool mounted;
    bool optionsValid;
    bool configurationValid;
    bool sourceValid;
    bool hostnameApplied;
    UmiOsBootOptions options;
    size_t configuredServices;
} UmiOsBootInputs;

typedef struct UmiOsBootDecision {
    const char *mode;
    const char *state;
    const char *reason;
    size_t planned;
    bool mayStartServices;
} UmiOsBootDecision;

/* PID1 owns and supplies each observed result. This function performs no I/O
 * and cannot independently attest that a mount or identity operation succeeded.
 * Failure takes precedence over requested recovery. Strings have static lifetime;
 * invalid arguments preserve output. Inputs and output must not overlap. */
bool UmiOsBootDecide(const UmiOsBootInputs *inputs, UmiOsBootDecision *out);
#endif
