#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "Fonts/fonts.h"
#include "GUI/GUI_Paint.h"
#include "e-Paper/EPD_2in13.h"
#include "app_disp_ctrl.h"

#define MAX_ISSUE_NUM   16

typedef struct
{
    unsigned char is_valid;
    time_t dead_time;
    char msg[1024];
}ISSUE;

typedef struct 
{
    int num;
    ISSUE issue[MAX_ISSUE_NUM];
}ISSUE_POOL;


ISSUE_POOL issue_pool;


static int get_ip(const char* interface, char* ip)
{
	int i=0;
	int sockfd;
	struct ifconf ifc;
	char buf[1024]={0};
	char ipbuf[20]={0};
	struct ifreq *ifr;
 
	ifc.ifc_len = 1024;
	ifc.ifc_buf = buf;
 
	if((sockfd = socket(AF_INET, SOCK_DGRAM,0))<0)
	{
	    printf("socket error\n");
		return -1;
	}
	ioctl(sockfd,SIOCGIFCONF, &ifc);
	ifr = (struct ifreq*)buf;
 
	for(i=(ifc.ifc_len/sizeof(struct ifreq)); i > 0; i--)
	{
		if (strncmp(interface, ifr->ifr_name, strlen(interface)) == 0)
		{
			inet_ntop(AF_INET,&((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr,ipbuf,20);
			strcpy(ip, ipbuf);
			return 0;
		}
		ifr = ifr +1;
	}
	return -1;
}

int is_mosquitto_alive()
{
    int count = 0;
    FILE* fp;
    char buf[64]; 
    char cmd[] = "ps -C mosquitto | wc -l";
    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        return -1;
    }

    if( (fgets(buf,64,fp))!= NULL ) 
    {
        count = atoi(buf); 
        if((count - 1) == 0) 
            return -1;
        else
            return 0;
    } 
    pclose(fp); 
}


void idle_issue()
{
    //ip
    char ip_eth0[32] = "eth0:";
	char ip_wlan0[32] = "wlan0:";

    if (get_ip("eth0", ip_eth0 + 5) != 0)
	{
		strcpy(ip_eth0, "eth0:disconnect");
	}

	if (get_ip("wlan0", ip_wlan0 + 6) != 0)
	{
		strcpy(ip_wlan0, "wlan0:disconnect");
	}


    //time
    char time_str[16];
    char* pstr = time_str;
	time_t timep;
	struct tm* ptm;
	
	time(&timep);
	ptm = localtime(&timep);
    if (ptm->tm_hour / 10 != 0)
    {
        *pstr = ptm->tm_hour / 10 + '0';
        pstr++;
    }
    *pstr = ptm->tm_hour % 10 + '0';
    pstr++;

    *pstr = ':';
    pstr++;

    *pstr = ptm->tm_min / 10 + '0';
    pstr++;
    *pstr = ptm->tm_min % 10 + '0';
    pstr++;

    *pstr = '\0';


    //find mosquitto
    char mosquitto[16] = "mosquitto:Y";
    if (is_mosquitto_alive() == 0)
        mosquitto[10] = 'Y';
    else
        mosquitto[10] = 'N';

    //set image


    GET_LOCK();
    select_my_image();
    Paint_Clear(WHITE);
    Paint_DrawString_EN(1, 1, "TiPi Version1.0", &Font24, WHITE, BLACK);
   
    Paint_DrawString_EN(1, 1+30, ip_eth0, &Font16, WHITE, BLACK);
    Paint_DrawString_EN(1, 1+30+16, ip_wlan0, &Font16, WHITE, BLACK);
    Paint_DrawString_EN(1, 1+30+16+16, mosquitto, &Font16, WHITE, BLACK);

     Paint_DrawString_EN(1, 1+16*6, time_str, &Font16, WHITE, BLACK);
    
    disp_my_image();
    RELEASE_LOCK();
}


void clean_issue()
{
    time_t timep;	
	time(&timep); 
    for (int i = 0; i < MAX_ISSUE_NUM; i++)
    {
        if (issue_pool.issue[i].is_valid == 0)
            continue;
        if (issue_pool.issue[i].dead_time <= timep)
        {
            issue_pool.issue[i].is_valid = 0;
            issue_pool.num--;
        }
            
    }
}


int add_issue(time_t life, char* msg)
{
    time_t timep;	
	time(&timep); 
    for (int i = 0; i < MAX_ISSUE_NUM; i++)
    {
        if (issue_pool.issue[i].is_valid == 0)
        {
            issue_pool.issue[i].is_valid = 1;
            issue_pool.issue[i].dead_time = timep + life;
            strcpy(issue_pool.issue[i].msg, msg);
            issue_pool.num++;
            return 0;
        }
    }
    return -1;
}

void issue_start()
{
    issue_pool.num = 0;
    for (int i = 0; i < MAX_ISSUE_NUM; i++)
    {
        issue_pool.issue[i].is_valid = 0;
    }
}


void issue_schedule()
{
    char* pstr;
    clean_issue();
    if (issue_pool.num == 0)
    {
        return ;
    }
    for (int i = 0; i < MAX_ISSUE_NUM; i++)
    {
        if (issue_pool.issue[i].is_valid)
        {
            pstr = issue_pool.issue[i].msg;
            disp(msg);
        }
    }

}