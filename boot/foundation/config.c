/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/foundation/config.c
 *
 * PURPOSE:
 *   Validate the complete service graph before any executable is started.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "boot.h"
#include <string.h>

static bool Token(const char *text, size_t length, char *out, size_t capacity)
{
    if (length == 0 || length >= capacity) return false;
    for (size_t i = 0; i < length; ++i) {
        unsigned char c = (unsigned char)text[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (i > 0 && c == '-')))
            return false;
    }
    if (text[length - 1] == '-') return false;
    memcpy(out, text, length);
    out[length] = 0;
    return true;
}
static bool Executable(const char *text, size_t length, char *out, size_t capacity)
{
    const char prefix[] = "/usr/libexec/umicom-";
    char tail[128];
    size_t start = sizeof prefix - 1;
    if (length <= start || length >= capacity || memcmp(text, prefix, start) != 0 ||
        !Token(text + start, length - start, tail, sizeof tail)) return false;
    memcpy(out, text, length);
    out[length] = 0;
    return true;
}
static bool Timeout(const char *text, size_t length, uint32_t *out)
{
    uint32_t number = 0;
    if (length == 0 || length > 5 || text[0] == '0') return false;
    for (size_t i = 0; i < length; ++i) {
        if (text[i] < '0' || text[i] > '9') return false;
        number = number * 10U + (uint32_t)(text[i] - '0');
    }
    if (number < 100U || number > 30000U) return false;
    *out = number;
    return true;
}
static bool Service(const char *line, size_t length, UmiOsService *out)
{
    const char *fields[4]; size_t sizes[4]; size_t start = 0, field = 0;
    for (size_t i = 0; i <= length; ++i) {
        if (i == length || line[i] == '|') {
            if (field == 4) return false;
            fields[field] = line + start; sizes[field++] = i - start; start = i + 1;
        }
    }
    if (field != 4 || !Token(fields[0], sizes[0], out->id, sizeof out->id) ||
        !Executable(fields[1], sizes[1], out->executable, sizeof out->executable) ||
        !Timeout(fields[3], sizes[3], &out->timeoutMs)) return false;
    if (sizes[2] == 1 && fields[2][0] == '-') out->dependency[0] = 0;
    else if (!Token(fields[2], sizes[2], out->dependency, sizeof out->dependency)) return false;
    return true;
}
static bool Order(UmiOsBootConfig *config)
{
    bool done[UMI_OS_SERVICE_MAX] = {0};
    for (size_t i = 0; i < config->count; ++i) {
        for (size_t j = 0; j < i; ++j)
            if (strcmp(config->services[i].id, config->services[j].id) == 0) return false;
        const char *dep = config->services[i].dependency;
        if (!dep[0]) continue;
        bool found = false;
        for (size_t j = 0; j < config->count; ++j)
            if (i != j && strcmp(dep, config->services[j].id) == 0) found = true;
        if (!found) return false;
    }
    for (size_t n = 0; n < config->count; ++n) {
        bool added = false;
        for (size_t i = 0; i < config->count && !added; ++i) {
            if (done[i]) continue;
            const char *dep = config->services[i].dependency;
            bool ready = dep[0] == 0;
            for (size_t j = 0; j < config->count; ++j)
                if (done[j] && strcmp(dep, config->services[j].id) == 0) ready = true;
            if (ready) { config->order[n] = i; done[i] = true; added = true; }
        }
        if (!added) return false;
    }
    return true;
}
bool UmiOsBootConfigParse(const char *data, size_t size, UmiOsBootConfig *out)
{
    static const char magic[] = "UMICOM_OS_BOOT 1\n";
    UmiOsBootConfig config = {0}; size_t cursor = sizeof magic - 1;
    if (!data || !out || size < cursor || size > UMI_OS_CONFIG_MAX ||
        memcmp(data, magic, cursor) != 0 || data[size - 1] != '\n') return false;
    for (size_t i = 0; i < size; ++i)
        if (((unsigned char)data[i] < 32U && data[i] != '\n') || (unsigned char)data[i] > 126U)
            return false;
    while (cursor < size) {
        size_t end = cursor;
        while (end < size && data[end] != '\n') ++end;
        size_t length = end - cursor; const char *line = data + cursor;
        if (length == 0 || line[0] == '#') { cursor = end + 1; continue; }
        if (length > 9 && memcmp(line, "hostname=", 9) == 0) {
            if (config.hostname[0] || !Token(line + 9, length - 9, config.hostname, sizeof config.hostname))
                return false;
        } else if (length > 8 && memcmp(line, "service=", 8) == 0) {
            if (config.count == UMI_OS_SERVICE_MAX ||
                !Service(line + 8, length - 8, &config.services[config.count])) return false;
            ++config.count;
        } else return false;
        cursor = end + 1;
    }
    if (!config.hostname[0] || config.count == 0 || !Order(&config)) return false;
    *out = config;
    return true;
}
bool UmiOsBootOptionsParse(const char *data, size_t size, UmiOsBootOptions *out)
{
    UmiOsBootOptions result = {0}; unsigned seen = 0; size_t cursor = 0;
    if (!data || !out || size > UMI_OS_CONFIG_MAX) return false;
    for (size_t i = 0; i < size; ++i)
        if (((unsigned char)data[i] < 32U && data[i] != '\n' && data[i] != '\t') ||
            (unsigned char)data[i] > 126U) return false;
    while (cursor < size) {
        while (cursor < size && (data[cursor] == ' ' || data[cursor] == '\n' || data[cursor] == '\t')) ++cursor;
        size_t end = cursor;
        while (end < size && data[end] != ' ' && data[end] != '\n' && data[end] != '\t') ++end;
        size_t length = end - cursor;
        if (length >= 7 && memcmp(data + cursor, "umicom.", 7) == 0) {
            unsigned bit; bool *value;
            const char *option;
            if (length >= 16 && memcmp(data + cursor, "umicom.recovery=", 16) == 0) {
                bit = 1U; value = &result.recovery; option = data + cursor + 16;
                if (length != 17) return false;
            } else if (length >= 21 && memcmp(data + cursor, "umicom.autopoweroff=", 20) == 0) {
                bit = 2U; value = &result.autopoweroff; option = data + cursor + 20;
                if (length != 21) return false;
            } else return false;
            if ((seen & bit) || (*option != '0' && *option != '1')) return false;
            seen |= bit; *value = *option == '1';
        }
        cursor = end;
    }
    *out = result;
    return true;
}
