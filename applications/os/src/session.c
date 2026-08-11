/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/session.c
 *
 * PURPOSE:
 *   Implement one product-level Umicom OS user-space service or model.
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
#include "umicom/os/session.h"
UmiOsSession umi_os_session_new(uint64_t id){UmiOsSession s={id,true};return s;}
