/*-----------------------------------------------------------------------------
 * Umicom OS tests
 * File: tests/foundation/test_config.c
 *
 * PURPOSE:
 *   Check service graph validation and command-line controls without booting.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "boot.h"
#include <stdio.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"check %d: %s\n",__LINE__,#e); return 1; } } while(0)
static const char GOOD[]="UMICOM_OS_BOOT 1\nhostname=umicom\nservice=one|/usr/libexec/umicom-one|-|1000\nservice=two|/usr/libexec/umicom-two|one|2000\n";
static bool Parse(const char *text) { UmiOsBootConfig c;return UmiOsBootConfigParse(text,strlen(text),&c); }
int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    const char *test=argv[1]; UmiOsBootConfig config; char text[5000];
    if(strcmp(test,"valid")==0) {
        CHECK(UmiOsBootConfigParse(GOOD,strlen(GOOD),&config)); CHECK(config.count==2 && config.order[0]==0 && config.order[1]==1);
    } else if(strcmp(test,"forward")==0) {
        strcpy(text,"UMICOM_OS_BOOT 1\nhostname=umicom\nservice=two|/usr/libexec/umicom-two|one|1000\nservice=one|/usr/libexec/umicom-one|-|2000\n");
        CHECK(UmiOsBootConfigParse(text,strlen(text),&config)); CHECK(config.order[0]==1 && config.order[1]==0);
    } else if(strcmp(test,"cycle")==0) {
        CHECK(!Parse("UMICOM_OS_BOOT 1\nhostname=umicom\nservice=one|/usr/libexec/umicom-one|two|1000\nservice=two|/usr/libexec/umicom-two|one|1000\n"));
    } else if(strcmp(test,"missing")==0) {
        CHECK(!Parse("UMICOM_OS_BOOT 1\nhostname=umicom\nservice=one|/usr/libexec/umicom-one|missing|1000\n"));
    } else if(strcmp(test,"duplicate")==0) {
        strcpy(text,GOOD);strcat(text,"service=one|/usr/libexec/umicom-other|-|1000\n");CHECK(!Parse(text));
    } else if(strcmp(test,"hostname")==0) {
        strcpy(text,GOOD);strcat(text,"hostname=other\n");CHECK(!Parse(text));
        CHECK(!Parse("UMICOM_OS_BOOT 1\nhostname=../bad\nservice=one|/usr/libexec/umicom-one|-|1000\n"));
    } else if(strcmp(test,"path")==0) {
        const char *paths[]={"/bin/sh","/usr/libexec/umicom-../bin","/usr/libexec/umicom-a;id","relative","/usr/libexec/umicom-"};
        for(size_t i=0;i<sizeof paths/sizeof paths[0];++i){snprintf(text,sizeof text,"UMICOM_OS_BOOT 1\nhostname=umicom\nservice=one|%s|-|1000\n",paths[i]);CHECK(!Parse(text));}
    } else if(strcmp(test,"timeout")==0) {
        const char *values[]={"0","99","30001","999999999999999999999","-1","+1000","1e3","01000"};
        for(size_t i=0;i<sizeof values/sizeof values[0];++i){snprintf(text,sizeof text,"UMICOM_OS_BOOT 1\nhostname=umicom\nservice=one|/usr/libexec/umicom-one|-|%s\n",values[i]);CHECK(!Parse(text));}
    } else if(strcmp(test,"count")==0) {
        strcpy(text,"UMICOM_OS_BOOT 1\nhostname=umicom\n");
        for(unsigned i=0;i<16;++i){char line[120];snprintf(line,sizeof line,"service=item%u|/usr/libexec/umicom-check|-|1000\n",i);strcat(text,line);}
        CHECK(Parse(text));strcat(text,"service=extra|/usr/libexec/umicom-check|-|1000\n");CHECK(!Parse(text));
    } else if(strcmp(test,"options")==0) {
        UmiOsBootOptions options;
        const char *yes="console=ttyS0 rdinit=/init umicom.recovery=1 umicom.autopoweroff=1\n";
        CHECK(UmiOsBootOptionsParse(yes,strlen(yes),&options));CHECK(options.recovery && options.autopoweroff);
        const char *bad[]={"umicom.recovery=2","umicom.recovery=1 umicom.recovery=0","umicom.autopoweroff=2","umicom.shell=1","umicom.recovery=","umicom.autopoweroff=10"};
        for(size_t i=0;i<sizeof bad/sizeof bad[0];++i)CHECK(!UmiOsBootOptionsParse(bad[i],strlen(bad[i]),&options));
        CHECK(UmiOsBootOptionsParse("",0,&options));CHECK(!options.recovery && !options.autopoweroff);
    } else if(strcmp(test,"malformed")==0) {
        CHECK(!Parse(""));CHECK(!Parse("UMICOM_OS_BOOT 2\n"));
        for(size_t i=0;i<strlen(GOOD);++i) {
            strcpy(text,GOOD);text[i]=0;CHECK(!UmiOsBootConfigParse(text,strlen(GOOD),&config));
        }
    } else if(strcmp(test,"transactional")==0) {
        memset(&config,0x55,sizeof config);UmiOsBootConfig old=config;
        CHECK(!UmiOsBootConfigParse("bad",3,&config));CHECK(memcmp(&old,&config,sizeof old)==0);
        CHECK(!UmiOsBootConfigParse(NULL,1,&config));CHECK(!UmiOsBootConfigParse(GOOD,strlen(GOOD),NULL));
    } else if(strcmp(test,"fuzz")==0) {
        uint32_t random=98765;
        for(size_t n=0;n<10000;++n){strcpy(text,GOOD);size_t size=strlen(text);random=random*1664525U+1013904223U;size_t pos=random%size;random=random*1664525U+1013904223U;text[pos]=(char)(random&255U);if(UmiOsBootConfigParse(text,size,&config))CHECK(config.count>0 && config.count<=UMI_OS_SERVICE_MAX);}
    } else return 2;
    return 0;
}
