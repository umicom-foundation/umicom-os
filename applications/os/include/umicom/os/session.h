/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/session.h
 *
 * PURPOSE:
 *   Represent a desktop user session without owning Linux authentication internals.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

/* BEGINNER NOTE:
 * This file keeps one responsibility small and explicit. Read the public
 * structure/function declarations first, then follow the implementation in
 * the matching source file.
 */
#ifndef UMICOM_OS_SESSION_H
#define UMICOM_OS_SESSION_H
#include <stdbool.h>
#include <stdint.h>
typedef struct UmiOsSession { uint64_t session_id; bool active; } UmiOsSession;
UmiOsSession umi_os_session_new(uint64_t id);
#endif
