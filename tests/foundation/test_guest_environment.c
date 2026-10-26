/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tests/foundation/test_guest_environment.c
 *
 * PURPOSE:
 *   Reject incomplete or unsafe guest observations without running privileged guest operations.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "guest_environment.h"
#include <stdio.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr, "line %d: %s\n", __LINE__, #e); return 1; } } while (0)

static UmiOsGuestEnvironment Valid(void)
{
    UmiOsGuestEnvironment e = {0};
    e.observed = UMI_OS_GUEST_ALL;
    e.uid = e.effectiveUid = e.gid = e.effectiveGid = 1000;
    e.noNewPrivileges = 1;
    e.fileLimit = e.fileHardLimit = 65536;
    e.descriptorLimit = e.descriptorHardLimit = 32;
    e.workUid = e.workGid = 1000;
    e.workMode = 0700;
    e.workDirectory = e.workingDirectoryMatches = true;
    e.proc = e.sys = e.run = e.tmp = (UmiOsGuestMount){true, true};
    return e;
}
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    UmiOsGuestEnvironment e = Valid();
    const char *name = argv[1];
    UmiOsGuestProblem expected = UMI_OS_GUEST_OK;
    if (strcmp(name, "valid") == 0) {}
    else if (strcmp(name, "argument") == 0) {
        CHECK(UmiOsGuestEnvironmentCheck(NULL) == UMI_OS_GUEST_ARGUMENT);
        return 0;
    } else if (strcmp(name, "missing-observation") == 0) {
        for (unsigned bit = 0; bit < 9; ++bit) {
            e = Valid(); e.observed &= ~(UINT32_C(1) << bit);
            CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_OBSERVATION);
        }
        e = Valid(); e.observed |= UINT32_C(1) << 31;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_OBSERVATION);
        return 0;
    } else if (strcmp(name, "identity") == 0) {
        uint64_t *ids[] = {&e.uid, &e.effectiveUid, &e.gid, &e.effectiveGid};
        for (size_t i = 0; i < sizeof ids / sizeof ids[0]; ++i) {
            *ids[i] = 0;
            CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_WRONG_IDENTITY);
            *ids[i] = 1000;
        }
        return 0;
    } else if (strcmp(name, "groups") == 0) {
        e.supplementaryGroups = -1;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_EXTRA_GROUPS);
        e.supplementaryGroups = 1; expected = UMI_OS_GUEST_EXTRA_GROUPS;
    } else if (strcmp(name, "privileges") == 0) {
        e.noNewPrivileges = 0; expected = UMI_OS_GUEST_PRIVILEGE_GAIN;
    } else if (strcmp(name, "core-limit") == 0) {
        e.coreLimit = UINT64_MAX; expected = UMI_OS_GUEST_UNBOUNDED_LIMITS;
    } else if (strcmp(name, "file-limit") == 0) {
        e.fileLimit = 0;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_UNBOUNDED_LIMITS);
        e.fileLimit = 65537; expected = UMI_OS_GUEST_UNBOUNDED_LIMITS;
    } else if (strcmp(name, "descriptor-limit") == 0) {
        e.descriptorLimit = 2;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_UNBOUNDED_LIMITS);
        e.descriptorLimit = 33; expected = UMI_OS_GUEST_UNBOUNDED_LIMITS;
    } else if (strcmp(name, "hard-limits") == 0) {
        e.coreHardLimit = 1;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_UNBOUNDED_LIMITS);
        e = Valid(); e.fileHardLimit = UINT64_MAX;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_UNBOUNDED_LIMITS);
        e = Valid(); e.descriptorHardLimit = 33;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_UNBOUNDED_LIMITS);
        e = Valid(); e.fileHardLimit = 1;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_UNBOUNDED_LIMITS);
        e = Valid(); e.descriptorHardLimit = 1; expected = UMI_OS_GUEST_UNBOUNDED_LIMITS;
    } else if (strcmp(name, "workspace-owner") == 0) {
        e.workUid = 0;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_UNSAFE_WORKSPACE);
        e.workUid = 1000; e.workGid = 0; expected = UMI_OS_GUEST_UNSAFE_WORKSPACE;
    } else if (strcmp(name, "workspace-mode") == 0) {
        const uint32_t modes[] = {0, 0600, 0750, 0777, 04700, 02700, 01700};
        for (size_t i = 0; i < sizeof modes / sizeof modes[0]; ++i) {
            e.workMode = modes[i];
            CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_UNSAFE_WORKSPACE);
        }
        return 0;
    } else if (strcmp(name, "workspace-location") == 0) {
        e.workDirectory = false;
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_UNSAFE_WORKSPACE);
        e.workDirectory = true; e.workingDirectoryMatches = false;
        expected = UMI_OS_GUEST_UNSAFE_WORKSPACE;
    } else if (strcmp(name, "mounts") == 0) {
        UmiOsGuestMount *mounts[] = {&e.proc, &e.sys, &e.run, &e.tmp};
        const UmiOsGuestProblem problems[] = {UMI_OS_GUEST_UNSAFE_PROC, UMI_OS_GUEST_UNSAFE_SYS,
            UMI_OS_GUEST_UNSAFE_RUN, UMI_OS_GUEST_UNSAFE_TMP};
        for (size_t i = 0; i < sizeof mounts / sizeof mounts[0]; ++i) {
            mounts[i]->expectedType = false;
            CHECK(UmiOsGuestEnvironmentCheck(&e) == problems[i]);
            mounts[i]->expectedType = true; mounts[i]->restricted = false;
            CHECK(UmiOsGuestEnvironmentCheck(&e) == problems[i]);
            mounts[i]->restricted = true;
        }
        return 0;
    } else if (strcmp(name, "unchanged") == 0) {
        unsigned char before[sizeof e]; memcpy(before, &e, sizeof e);
        CHECK(UmiOsGuestEnvironmentCheck(&e) == UMI_OS_GUEST_OK);
        CHECK(memcmp(before, &e, sizeof e) == 0);
        return 0;
    } else if (strcmp(name, "messages") == 0) {
        for (int i = UMI_OS_GUEST_OK; i <= UMI_OS_GUEST_UNSAFE_TMP; ++i)
            CHECK(strlen(UmiOsGuestProblemText((UmiOsGuestProblem)i)) > 0);
        CHECK(strcmp(UmiOsGuestProblemText((UmiOsGuestProblem)99), "unknown-environment-problem") == 0);
        return 0;
    } else return 2;
    CHECK(UmiOsGuestEnvironmentCheck(&e) == expected);
    return 0;
}
