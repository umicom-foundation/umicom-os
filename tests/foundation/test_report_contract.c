/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tests/foundation/test_report_contract.c
 *
 * PURPOSE:
 *   Check that independent guest reports remain readable through Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* The test links both sides only to check the file protocol. Guest init itself
 * never links Framework, including when Framework's normal probe cannot start. */
#define _POSIX_C_SOURCE 200809L
#include "boot.h"
#include "boot_admission.h"
#include "umicom/platform/boot_report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"contract line %d: %s\n",__LINE__,#e); return 1; } } while (0)
int main(void)
{
    char folder[]="/tmp/umicom-report-contract-XXXXXX";
    CHECK(mkdtemp(folder)!=NULL); CHECK(chdir(folder)==0);
    const char *id="0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    const char *modes[]={"normal","recovery"};
    const char *states[]={"starting","ready","recovery"};
    const char *reasons[]={"none","requested","configuration","mount","service-exit","service-timeout","service-launch","report-write","privilege"};
    size_t accepted=0;
    for(size_t m=0;m<2;++m) for(size_t s=0;s<3;++s) for(size_t r=0;r<9;++r)
        for(size_t planned=0;planned<=3;++planned) for(size_t completed=0;completed<=planned;++completed) {
            bool written=UmiOsWriteReport("report",modes[m],states[s],planned,completed,reasons[r],id);
            char text[512];
            int size=snprintf(text,sizeof text,"UMICOM_BOOT_REPORT 1\nmode=%s\nstate=%s\nplanned=%zu\ncompleted=%zu\nreason=%s\nsource=%s\n",modes[m],states[s],planned,completed,reasons[r],id);
            CHECK(size>0&&(size_t)size<sizeof text);
            UmiBootReport report;
            bool parsed=UmiBootReportParse(text,(size_t)size,&report)==UMI_STATUS_OK;
            CHECK(written==parsed);
            if(written){
                FILE *file=fopen("report","rb");CHECK(file!=NULL);char actual[512];size_t got=fread(actual,1,sizeof actual,file);CHECK(!ferror(file)&&fclose(file)==0);
                CHECK(got==(size_t)size&&memcmp(actual,text,got)==0);++accepted;
            }
        }

    /* Exercise the same policy PID1 uses, then check the independent writer and
     * Framework reader agree. This is a host contract, not a guest boot result. */
    for (unsigned mask = 0; mask < 64U; ++mask) {
        UmiOsBootInputs inputs = {
            (mask & 1U) != 0, (mask & 2U) != 0, (mask & 4U) != 0,
            (mask & 8U) != 0, (mask & 16U) != 0, {(mask & 32U) != 0, false}, 2
        };
        UmiOsBootDecision decision;
        CHECK(UmiOsBootDecide(&inputs, &decision));
        CHECK(UmiOsWriteReport("report", decision.mode, decision.state,
            decision.planned, 0, decision.reason, id));
        FILE *file = fopen("report", "rb");
        CHECK(file != NULL);
        char text[512];
        size_t size = fread(text, 1, sizeof text, file);
        CHECK(!ferror(file) && fclose(file) == 0);
        UmiBootReport report;
        CHECK(UmiBootReportParse(text, size, &report) == UMI_STATUS_OK);
    }
    CHECK(accepted>0);CHECK(unlink("report")==0);CHECK(chdir("/")==0);CHECK(rmdir(folder)==0);
    printf("Checked 540 protocol combinations (%zu valid) and 64 PID1 admission decisions.\n",accepted);
    return 0;
}
