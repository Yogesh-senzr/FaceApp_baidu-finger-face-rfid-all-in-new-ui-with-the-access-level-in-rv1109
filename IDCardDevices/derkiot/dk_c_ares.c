#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <stdio.h>
#include "ares.h"

#define IP_LEN 32

typedef struct {
    char host[64];
    char ip[10][IP_LEN];
    int count;
}IpList;

void dns_callback (void* arg, int status, int timeouts, struct hostent* hptr)  //ares  处理完成，返回DNS解析的信息
{
    IpList *ips = (IpList*)arg;
    if( ips == NULL ) return;
    if(status == ARES_SUCCESS){
        strncpy(ips->host, hptr->h_name, sizeof(ips->host));
        char **pptr=hptr->h_addr_list;
        for(int i=0; *pptr!=NULL && i<10; pptr++,++i){
            inet_ntop(hptr->h_addrtype, *pptr, ips->ip[ips->count++], IP_LEN);
        }
    }else{
        printf("lookup failed: %d", status);
    }
}

//static int CloudServerHostIsIP(const char * serverhost)
//{
//	struct in_addr addr;
//	int lsuccess;
//	lsuccess= inet_pton(AF_INET, serverhost, &addr);
//	return lsuccess> 0 ? 0 : -1;
//}

static void DNSCallBack(void* arg, int status, int timeouts, struct hostent* host)
{
    char **lpSrc;
    char  * lpHost = (char *)arg;

    if (status == ARES_SUCCESS)
    {
        for (lpSrc = host->h_addr_list; *lpSrc; lpSrc++)
        {
            char addr_buf[32] = "";
            ares_inet_ntop(host->h_addrtype, *lpSrc, addr_buf, sizeof(addr_buf));
            if (strlen(addr_buf) != 0)
            {
                strcpy(lpHost, addr_buf);
                break;
            }
        }
    }
}

static int DomainNameReSolution(const char * lpDomainName, char * lpHost)
{

    int lsuccess= 0;

    ares_channel channel;
    lsuccess= ares_library_init(ARES_LIB_INIT_ALL);
    if ((lsuccess= ares_init(&channel)) != ARES_SUCCESS) return -1;

    int trytime = 1;
    do{
        fd_set readers, writers;
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        FD_ZERO(&readers);
        FD_ZERO(&writers);
        ares_gethostbyname(channel, lpDomainName, AF_INET, DNSCallBack, (char *)lpHost);
        int nfds = ares_fds(channel, &readers, &writers);
        if (nfds == 0){
            continue;
        }
        int count = select(nfds, &readers, NULL, NULL, &tv);
        if (count > 0){
            ares_process(channel, &readers, &writers);
            lsuccess= 0;
            break;
        }
        else{
            lsuccess= -1;
        }
    }while (trytime-- > 0);

    ares_destroy(channel);
    ares_library_cleanup();
    return lsuccess;
}



int dk_gethostbyname(char *host, char *ip)
{
    int res;

    if(host == NULL || ip == NULL) {
        return -1;
    }
    res = DomainNameReSolution(host, ip);
    printf("dns analysis end..\r\n");
    if(res == 0) {
        return 1;
    }
    return -1;
}

