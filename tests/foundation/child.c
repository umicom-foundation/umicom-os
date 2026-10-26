/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: tests/foundation/child.c
 *
 * PURPOSE:
 *   Supply real child processes for descriptor, identity, resource and deadline tests.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <unistd.h>
int main(int argc,char **argv)
{
    (void)argc;const char *name=strrchr(argv[0],'/');name=name?name+1:argv[0];
    if(strcmp(name,"failed")==0)return 7;
    if(strcmp(name,"signal")==0){raise(SIGTERM);return 1;}
    if(strcmp(name,"timeout")==0){sleep(10);return 0;}
    if(strcmp(name,"childgroup")==0){pid_t pid=fork();if(pid<0)return 1;if(pid==0){sleep(2);int f=open("escaped-marker",O_WRONLY|O_CREAT,0600);if(f>=0)close(f);_exit(0);}return 0;}
    if(strcmp(name,"descriptors")==0){for(int f=3;f<80;++f)if(fcntl(f,F_GETFD)>=0)return 1;return 0;}
    if(strcmp(name,"limits")==0){struct rlimit r;if(getrlimit(RLIMIT_CORE,&r)!=0||r.rlim_cur!=0)return 1;if(getrlimit(RLIMIT_FSIZE,&r)!=0||r.rlim_cur>65536)return 2;if(prctl(PR_GET_NO_NEW_PRIVS,0,0,0,0)!=1)return 3;return 0;}
    /* Preserve the original identity-only check. The replacement checks the
     * effective group, supplementary groups and privilege restriction applied
     * by the real supervisor inside the existing private chroot harness. */
#if 0
    if(strcmp(name,"identity")==0)return getuid()==1000&&geteuid()==1000&&getgid()==1000?0:1;
#endif
    if(strcmp(name,"identity")==0){
        if(getuid()!=1000||geteuid()!=1000||getgid()!=1000||getegid()!=1000)return 1;
        if(getgroups(0,NULL)!=0||prctl(PR_GET_NO_NEW_PRIVS,0,0,0,0)!=1)return 2;
        struct rlimit memory,processes;
        if(getrlimit(RLIMIT_AS,&memory)!=0||memory.rlim_cur!=96U*1024U*1024U||memory.rlim_max!=memory.rlim_cur)return 3;
        if(getrlimit(RLIMIT_NPROC,&processes)!=0||processes.rlim_cur!=32||processes.rlim_max!=32)return 4;
        char current[128];
        if(!getcwd(current,sizeof current)||strcmp(current,"/run/umicom/work")!=0)return 5;
        return 0;
    }
    puts("Child completed successfully.");return 0;
}
