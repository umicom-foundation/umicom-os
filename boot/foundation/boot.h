/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/boot.h
 *
 * PURPOSE:
 *   Keep the minimal boot controller independent of Framework and GTK.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_OS_FOUNDATION_BOOT_H
#define UMICOM_OS_FOUNDATION_BOOT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define UMI_OS_SERVICE_MAX 16U
#define UMI_OS_CONFIG_MAX 4096U
#define UMI_OS_LOG_MAX (64U * 1024U)
#define UMI_OS_SOURCE_ID_SIZE 65U

typedef struct UmiOsService {
    char id[32];
    char executable[160];
    char dependency[32];
    uint32_t timeoutMs;
} UmiOsService;
typedef struct UmiOsBootConfig {
    char hostname[64];
    UmiOsService services[UMI_OS_SERVICE_MAX];
    size_t count;
    size_t order[UMI_OS_SERVICE_MAX];
} UmiOsBootConfig;
typedef struct UmiOsBootOptions {
    bool recovery;
    bool autopoweroff;
} UmiOsBootOptions;
typedef enum UmiOsServiceState {
    UMI_OS_SERVICE_OK = 0,
    UMI_OS_SERVICE_EXIT = 1,
    UMI_OS_SERVICE_TIMEOUT = 2,
    UMI_OS_SERVICE_LAUNCH = 3,
    UMI_OS_SERVICE_PRIVILEGE = 4
} UmiOsServiceState;
typedef struct UmiOsServiceResult {
    UmiOsServiceState state;
    int exitCode;
    int signalNumber;
    int launchError;
    uint64_t elapsedMs;
} UmiOsServiceResult;

/* Transactional parsing: an invalid input does not modify the caller's result.
 * Identifiers and paths use an intentionally narrow, non-shell grammar. */
bool UmiOsBootConfigParse(const char *data, size_t size, UmiOsBootConfig *out);
bool UmiOsBootOptionsParse(const char *data, size_t size, UmiOsBootOptions *out);
/* Caller serialises runs. Launches a single executable with no argument parsing
 * or shell. A private process group contains descendants. dropIdentity is true
 * in init and can be false only in the separate host test harness. */
UmiOsServiceResult UmiOsServiceRun(const UmiOsService *service, int logFd, bool dropIdentity);
const char *UmiOsServiceReason(UmiOsServiceState state);
/* Privileged I/O is only used by the PID-1 executable, never by parser tests. */
bool UmiOsWriteReport(const char *path, const char *mode, const char *state,
    size_t planned, size_t completed, const char *reason, const char *sourceId);
void UmiOsRecoveryConsole(const UmiOsBootConfig *config);
#endif
