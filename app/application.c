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
#include "application.h"


#define MAX_ISSUE_NUM   16

typedef enum  {TIP = 0, NOTE} ISSUE_TYPE;


typedef struct 
{
    int num;
    ISSUE issue[MAX_ISSUE_NUM];
}ISSUE_POOL;


ISSUE_POOL tips_pool;
ISSUE_POOL notes_pool;


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
            close(sockfd);
			return 0;
		}
		ifr = ifr +1;
	}
    close(sockfd);
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


struct last_status
{
    char ip_eth0[32];
    char ip_wlan0[32];
    int is_mosquitto;
    int ts; //ts = hour*60+min
};


void idle_issue()
{
    int if_update = 0;
    static struct last_status last = {"0","0",0,0};

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
    int this_mosquitto;
    char mosquitto[16] = "mosquitto:Y";
    if (is_mosquitto_alive() == 0)
    {
        this_mosquitto = 1;
        mosquitto[10] = 'Y';
    }   
    else
    {
        this_mosquitto = 0;
        mosquitto[10] = 'N';
    }
        

    //if update
    if (strcmp(ip_eth0, last.ip_eth0) != 0)
    {
        if_update = 1;
        strcpy(last.ip_eth0, ip_eth0);
    }

    if (strcmp(ip_wlan0, last.ip_wlan0) != 0)
    {
        if_update = 1;
        strcpy(last.ip_wlan0, ip_wlan0);
    }

    if (this_mosquitto != last.is_mosquitto)
    {
        if_update = 1;
        last.is_mosquitto = this_mosquitto;
    }

    if (ptm->tm_hour * 60 + ptm->tm_min != last.ts)
    {
        if_update = 1;
        last.ts = ptm->tm_hour * 60 + ptm->tm_min;
    }


    if (!if_update)
        return ;


    //set image
    GET_LOCK();
    select_my_image();
    Paint_Clear(WHITE);
    Paint_DrawString_EN(1, 1, "TiPi Ver1.0", &Font24, WHITE, BLACK);
   
    Paint_DrawString_EN(1, 1+30, ip_eth0, &Font16, WHITE, BLACK);
    Paint_DrawString_EN(1, 1+30+16, ip_wlan0, &Font16, WHITE, BLACK);
    Paint_DrawString_EN(1, 1+30+16+16, mosquitto, &Font16, WHITE, BLACK);

     Paint_DrawString_EN(1, 1+16*6, time_str, &Font16, WHITE, BLACK);
    
    disp_my_image();
    sleep(3);
    RELEASE_LOCK();
}


void clean_issue(ISSUE_TYPE type)
{
    ISSUE_POOL* pool;
    if (type == TIP)
        pool = &tips_pool;
    else if (type == NOTE)
        pool = &notes_pool;
    else 
        return ;
    
    time_t timep;	
	time(&timep); 
    for (int i = 0; i < MAX_ISSUE_NUM; i++)
    {
        if (pool->issue[i].is_valid == 0)
            continue;
        if (pool->issue[i].dead_time <= timep)
        {
            pool->issue[i].is_valid = 0;
            pool->num--;
        }     
    }
}


int add_issue(time_t life_sec, char* msg, ISSUE_TYPE type)
{
    ISSUE_POOL* p;
    if (type == TIP)
        p = &tips_pool;
    else if (type == NOTE)
        p = &notes_pool;
    else 
        return ;
    time_t timep;	
	time(&timep); 
    for (int i = 0; i < MAX_ISSUE_NUM; i++)
    {
        if (p->issue[i].is_valid == 0)
        {
            p->issue[i].is_valid = 1;
            p->issue[i].dead_time = timep + life_sec;
            strcpy(p->issue[i].msg, msg);
            p->num++;
            return 0;
        }
    }
    return -1;
}

void issue_start()
{
    ISSUE_POOL* p;
    p = &tips_pool;
    p->num = 0;
    for (int i = 0; i < MAX_ISSUE_NUM; i++)
        p->issue[i].is_valid = 0;
    
    p = &notes_pool;
    p->num = 0;
    for (int i = 0; i < MAX_ISSUE_NUM; i++)
        p->issue[i].is_valid = 0;
    
    return ;
}


void tips_merge(char* tips_str)
{
    char* p = tips_str;
    for (int i = 0; i < MAX_ISSUE_NUM; i++)
    {
        if (tips_pool.issue[i].is_valid)
        {
            char* msg = tips_pool.issue[i].msg;
            strncpy(p, msg, strlen(msg));
            p += strlen(msg);
            *p = '\n';
            p++;
        }
    }
    p--;
    *p = '\0';
}

int str_to_int(char* str)
{
    int data = 0;
    int pow = 1;
    for (int i = strlen(str)-1; i >= 0; i--)
    {
        data += (str[i] - '0')*pow;
        pow *= 10;
    } 
    return data;  
}


int parse_msg(char* msg, char* out1, char* out2)
{
    char* p1 = strstr(msg, "{");
    if (p1 == NULL)
        return -1;
    char* p2 = strstr(msg, "#");
    if (p2 == NULL)
        return -1;
    char* p3 = strstr(msg, "}");
    if(p3 == NULL)
        return -1;
    
    if ((int)p2 - (int)p1 <= 1)
        return -1;
    if ((int)p3 - (int)p2 <= 1)
        return -1;
    

    strncpy(out1, (int)p1+1, (int)p2 - (int)p1 - 1);
    *(out1 + (int)p2 - (int)p1 - 1) = '\0';
    strncpy (out2, (int)p2+1, (int)p3 - (int)p2 - 1);
    *(out2 + (int)p3 - (int)p2 - 1) = '\0';
    return 0;

}

void add_tip(char* msg)
{
    char data[1024];
    char life[64];
    if (parse_msg(msg, data, life) != 0)
        return ;
    int sec = str_to_int(life);
    add_issue(sec, data, TIP);
}


void add_note(char* msg)
{
    char data[1024];
    char life[64];
    if (parse_msg(msg, data, life) != 0)
        return ;
    int sec = str_to_int(life);
    add_issue(sec, data, NOTE);
}


int issue_schedule()
{
    idle_issue();
    clean_issue(TIP);
    clean_issue(NOTE);
    if (tips_pool.num == 0 && notes_pool.num == 0)
    {
	    EPD_2IN13_Init(EPD_2IN13_PART);
        return -1;
    }
    int issue_total = tips_pool.num + notes_pool.num;
    int time_piece = 60/issue_total;
    EPD_2IN13_Init(EPD_2IN13_FULL);
    char str_disp[1024];
    memset(str_disp, 0, 1024);
    int tip_num;
    if (tips_pool.num != 0)
    {
        tips_merge(str_disp);
        disp(str_disp);
        sleep(time_piece);
    }

    if (notes_pool.num != 0)
    {
        memset(str_disp, 0, 1024);
        for (int i = 0; i < MAX_ISSUE_NUM; i++)
        {
            if (notes_pool.issue[i].is_valid)
            {
                disp(notes_pool.issue[i].msg);
                sleep(time_piece);
            }
        }
    }  
    return 0;  
}