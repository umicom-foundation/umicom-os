/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/admission.c
 *
 * PURPOSE:
 *   Keep failure precedence consistent between PID1 and host regression checks.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "boot_admission.h"

bool UmiOsBootDecide(const UmiOsBootInputs *inputs, UmiOsBootDecision *out)
{
    if (!inputs || !out || inputs->configuredServices > UMI_OS_SERVICE_MAX)
        return false;

    /* Unaccepted options cannot select recovery. Admission belongs to the
     * minimal OS boot controller so recovery has no Framework dependency. */
    bool requested = inputs->optionsValid && inputs->options.recovery;
    UmiOsBootDecision result = {
        requested ? "recovery" : "normal", "recovery", "configuration",
        inputs->configurationValid ? inputs->configuredServices : 0U, false
    };
    if (!inputs->mounted) result.reason = "mount";
    else if (!inputs->optionsValid || !inputs->configurationValid ||
        !inputs->sourceValid || !inputs->hostnameApplied || !result.planned) {
        result.reason = "configuration";
    } else if (requested) result.reason = "requested";
    else {
        result.state = "starting";
        result.reason = "none";
        result.mayStartServices = true;
    }
    *out = result;
    return true;
}
