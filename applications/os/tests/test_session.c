/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/tests/test_session.c
 *
 * PURPOSE:
 *   Test one first-stage Umicom OS user-space boundary or model.
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
int main(void){UmiOsSession s=umi_os_session_new(7U);return s.active&&s.session_id==7U?0:1;}
