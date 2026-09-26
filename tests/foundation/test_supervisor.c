/*-----------------------------------------------------------------------------
 * Umicom OS tests
 * File: tests/foundation/test_supervisor.c
 *
 * PURPOSE:
 *   Test actual fork, exec, deadline and recovery-report behaviour in private directories.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#define _GNU_SOURCE
#include "boot.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(e) do { if(!(e)){fprintf(stderr,"check %d: %s errno=%d\n",__LINE__,#e,errno);return 1;} }while(0)
static int Copy(const char *source,const char *target)
{
    int in=open(source,O_RDONLY),out=open(target,O_WRONLY|O_CREAT|O_EXCL,0755);char buffer[4096];
    if(in<0||out<0)return 1;
    ssize_t n;while((n=read(in,buffer,sizeof buffer))>0)if(write(out,buffer,(size_t)n)!=n)return 1;
    close(in);close(out);return n<0?1:0;
}
int main(int argc,char **argv)
{
    if(argc!=3)return 2;
    const char *test=argv[1];char directory[]="/tmp/umicom-os-test-XXXXXX";CHECK(mkdtemp(directory));CHECK(chdir(directory)==0);
    UmiOsService service={0};service.timeoutMs=1500;
    int length=snprintf(service.executable,sizeof service.executable,"%s/%s",directory,test);CHECK(length>0&&(size_t)length<sizeof service.executable);
    int log=open("service.log",O_WRONLY|O_CREAT|O_EXCL,0600);CHECK(log>=0);
    if(strcmp(test,"invalid")==0){service.timeoutMs=0;CHECK(UmiOsServiceRun(&service,log,false).state==UMI_OS_SERVICE_LAUNCH);return 0;}
    if(strcmp(test,"report")==0){const char *id="0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";CHECK(UmiOsWriteReport("report","normal","ready",2,2,"none",id));CHECK(UmiOsWriteReport("report","normal","recovery",2,1,"service-exit",id));FILE *f=fopen("report","rb");CHECK(f);char b[512]={0};CHECK(fread(b,1,sizeof b-1,f)>0);fclose(f);CHECK(strstr(b,"state=recovery\n"));CHECK(!UmiOsWriteReport("report","normal","ready",17,17,"none",id));CHECK(!UmiOsWriteReport("report","normal","ready",2,1,"none",id));CHECK(!UmiOsWriteReport("report","normal","recovery",2,1,"x\ny",id));CHECK(!UmiOsWriteReport("report","recovery","starting",2,0,"none",id));return 0;}
    if(strcmp(test,"identity")==0){
        /* A chroot test never modifies host /run, users, mounts or credentials.
         * It uses only a static test executable and a regular empty stdin file. */
        if(geteuid()!=0)return 77;
#ifdef UMICOM_TEST_SANITIZED
        return 77;
#endif
        CHECK(chmod(directory,0755)==0);
        CHECK(mkdir("run",0755)==0&&mkdir("run/umicom",0755)==0&&mkdir("run/umicom/work",0700)==0);
        CHECK(chown("run/umicom/work",1000,1000)==0);CHECK(mkdir("dev",0755)==0);
        int nullFd=open("dev/null",O_WRONLY|O_CREAT,0644);CHECK(nullFd>=0);close(nullFd);
        CHECK(Copy(argv[2],"identity")==0);
        pid_t child=fork();CHECK(child>=0);
        if(child==0){
            if(chroot(directory)!=0||chdir("/")!=0)_exit(77);
            strcpy(service.executable,"/identity");
            UmiOsServiceResult r=UmiOsServiceRun(&service,log,true);
            if(r.state==UMI_OS_SERVICE_LAUNCH&&r.launchError==EAGAIN){
                dprintf(2,"SKIP: the host per-UID process quota prevents execution after switching to UID 1000. The guest quota is not relaxed.\n");
                _exit(77);
            }
            if(r.state!=UMI_OS_SERVICE_OK)dprintf(2,"identity state=%d error=%d exit=%d signal=%d\n",r.state,r.launchError,r.exitCode,r.signalNumber);
            _exit(r.state==UMI_OS_SERVICE_OK?0:1);
        }
        int status;CHECK(waitpid(child,&status,0)==child);CHECK(WIFEXITED(status));return WEXITSTATUS(status);
    }
    if(strcmp(test,"exec")!=0){
        if(strcmp(test,"format")==0){int fd=open(service.executable,O_WRONLY|O_CREAT|O_EXCL,0755);CHECK(fd>=0);CHECK(write(fd,"not an executable\n",18)==18);close(fd);}
        else CHECK(symlink(argv[2],service.executable)==0);
    }
    if(strcmp(test,"timeout")==0)service.timeoutMs=100;
    int high=-1;
    if(strcmp(test,"descriptors")==0){high=fcntl(log,F_DUPFD,50);CHECK(high>=50);}
    if(strcmp(test,"childgroup")==0)CHECK(prctl(PR_SET_CHILD_SUBREAPER,1,0,0,0)==0);
    UmiOsServiceResult result=UmiOsServiceRun(&service,log,false);
    if(high>=0)close(high);
    if(strcmp(test,"failed")==0){CHECK(result.state==UMI_OS_SERVICE_EXIT&&result.exitCode==7);}
    else if(strcmp(test,"signal")==0){CHECK(result.state==UMI_OS_SERVICE_EXIT&&result.signalNumber==SIGTERM);}
    else if(strcmp(test,"timeout")==0){CHECK(result.state==UMI_OS_SERVICE_TIMEOUT&&result.elapsedMs<2500);}
    else if(strcmp(test,"exec")==0){CHECK(result.state==UMI_OS_SERVICE_LAUNCH&&result.launchError==ENOENT);}
    else if(strcmp(test,"format")==0){CHECK(result.state==UMI_OS_SERVICE_LAUNCH&&result.launchError==ENOEXEC);}
    else {CHECK(result.state==UMI_OS_SERVICE_OK);}
    if(strcmp(test,"childgroup")==0){sleep(3);CHECK(access("escaped-marker",F_OK)!=0);int status;while(waitpid(-1,&status,WNOHANG)>0){}}
    close(log);return 0;
}
