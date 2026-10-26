/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tests/foundation/test_admission.c
 *
 * PURPOSE:
 *   Check that startup prerequisites and requested recovery preserve failure precedence.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "boot_admission.h"
#include <stdio.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr, "admission %d: %s\n", __LINE__, #e); return 1; } } while (0)

static UmiOsBootInputs Healthy(void)
{
    UmiOsBootInputs value = {true, true, true, true, true, {false, false}, 2};
    return value;
}
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    UmiOsBootInputs inputs = Healthy();
    UmiOsBootDecision decision;
    if (!strcmp(argv[1], "normal")) {
        CHECK(UmiOsBootDecide(&inputs, &decision));
        CHECK(decision.mayStartServices && decision.planned == 2);
        CHECK(!strcmp(decision.mode, "normal") && !strcmp(decision.state, "starting"));
        CHECK(!strcmp(decision.reason, "none"));
    } else if (!strcmp(argv[1], "recovery")) {
        inputs.options.recovery = true;
        CHECK(UmiOsBootDecide(&inputs, &decision));
        CHECK(!decision.mayStartServices && decision.planned == 2);
        CHECK(!strcmp(decision.mode, "recovery") && !strcmp(decision.state, "recovery"));
        CHECK(!strcmp(decision.reason, "requested"));
    } else if (!strcmp(argv[1], "hostname-recovery")) {
        inputs.options.recovery = true;
        inputs.hostnameApplied = false;
        CHECK(UmiOsBootDecide(&inputs, &decision));
        CHECK(!decision.mayStartServices && !strcmp(decision.reason, "configuration"));
        CHECK(!strcmp(decision.mode, "recovery") && !strcmp(decision.state, "recovery"));
    } else if (!strcmp(argv[1], "mount")) {
        inputs.options.recovery = true;
        inputs.mounted = false;
        inputs.sourceValid = false;
        CHECK(UmiOsBootDecide(&inputs, &decision));
        CHECK(!decision.mayStartServices && !strcmp(decision.reason, "mount"));
    } else if (!strcmp(argv[1], "untrusted-options")) {
        inputs.options.recovery = true;
        inputs.optionsValid = false;
        CHECK(UmiOsBootDecide(&inputs, &decision));
        CHECK(!decision.mayStartServices && !strcmp(decision.mode, "normal"));
        CHECK(!strcmp(decision.reason, "configuration"));
    } else if (!strcmp(argv[1], "empty")) {
        inputs.configuredServices = 0;
        CHECK(UmiOsBootDecide(&inputs, &decision));
        CHECK(!decision.mayStartServices && decision.planned == 0);
        CHECK(!strcmp(decision.reason, "configuration"));
    } else if (!strcmp(argv[1], "bounds") || !strcmp(argv[1], "arguments")) {
        CHECK(UmiOsBootDecide(&inputs, &decision));
        UmiOsBootDecision saved;
        memcpy(&saved, &decision, sizeof saved);
        if (!strcmp(argv[1], "bounds")) {
            inputs.configuredServices = UMI_OS_SERVICE_MAX + 1U;
            CHECK(!UmiOsBootDecide(&inputs, &decision));
        } else {
            CHECK(!UmiOsBootDecide(NULL, &decision));
            CHECK(!UmiOsBootDecide(&inputs, NULL));
        }
        CHECK(!memcmp(&saved, &decision, sizeof saved));
    } else if (!strcmp(argv[1], "matrix")) {
        /* Every prerequisite is varied independently. A requested recovery
         * never schedules normal services, even when every prerequisite passes. */
        size_t admitted = 0;
        for (unsigned mask = 0; mask < 128U; ++mask) {
            inputs = Healthy();
            inputs.mounted = (mask & 1U) != 0;
            inputs.optionsValid = (mask & 2U) != 0;
            inputs.configurationValid = (mask & 4U) != 0;
            inputs.sourceValid = (mask & 8U) != 0;
            inputs.hostnameApplied = (mask & 16U) != 0;
            inputs.options.recovery = (mask & 32U) != 0;
            inputs.options.autopoweroff = (mask & 64U) != 0;
            CHECK(UmiOsBootDecide(&inputs, &decision));
            if (decision.mayStartServices) {
                ++admitted;
                CHECK((mask & 63U) == 31U);
                CHECK(!strcmp(decision.state, "starting"));
            } else CHECK(!strcmp(decision.state, "recovery"));
            if (!inputs.configurationValid) CHECK(decision.planned == 0);
            if (!strcmp(decision.reason, "requested")) CHECK((mask & 63U) == 63U);
        }
        CHECK(admitted == 2U); /* autopoweroff does not authorize startup */
    } else return 2;
    return 0;
}
