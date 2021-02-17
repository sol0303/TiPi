#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <time.h>
#include "Fonts/fonts.h"
#include "GUI/GUI_Paint.h"
#include "e-Paper/EPD_2in13.h"
#include "app_mqtt.h"
int get_ip(const char* interface, char* ip)
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

UBYTE* BlackImage = NULL;
int init_screen()
{
	if (DEV_Module_Init() != 0)
	{
		printf("init dev fail\n");
		return -1;
	}
	EPD_2IN13_Init(EPD_2IN13_FULL);
	EPD_2IN13_Clear();
	EPD_2IN13_Init(EPD_2IN13_PART);
 	EPD_2IN13_Clear();
	usleep(500000);
	UWORD Imagesize = ((EPD_2IN13_WIDTH % 8 == 0)?(EPD_2IN13_WIDTH / 8) : (EPD_2IN13_WIDTH / 8 + 1)) * EPD_2IN13_HEIGHT;
	BlackImage = (UBYTE*)malloc(Imagesize);
	if (BlackImage  == NULL)
	{
		printf("malloc fail\n");
		return -1;
	}
	Paint_NewImage(BlackImage, EPD_2IN13_WIDTH, EPD_2IN13_HEIGHT, 270, WHITE);
	Paint_SelectImage(BlackImage);
	Paint_Clear(WHITE);
	Paint_DrawString_EN(15, 15, "eth0:", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(15, 35, "wlan0:", &Font16, BLACK, WHITE);
	EPD_2IN13_Display(BlackImage);
	usleep(2000000);
	return 0;
}


int disp_ip(const char* eth0, const char* wlan0)
{
	Paint_SelectImage(BlackImage);
	Paint_DrawString_EN(15 + 11*5, 15, eth0, &Font16, BLACK, WHITE);
	Paint_DrawString_EN(15 + 11*6, 35, wlan0, &Font16, BLACK, WHITE);
	EPD_2IN13_Display(BlackImage);
	return 0;
}

int disp_time(PAINT_TIME* time)
{
	Paint_SelectImage(BlackImage);
	Paint_ClearWindows(15, 70, 15 + Font16.Width * 7, 70 + Font16.Height, WHITE);
	Paint_DrawTime(15, 70, time, &Font16, WHITE, BLACK);
	EPD_2IN13_Display(BlackImage);
	return 0;
}

int update_ip()
{

	char ip_eth0[32];
	char ip_wlan0[32];
	if (get_ip("eth0", ip_eth0) != 0)
	{
		memset(ip_eth0, 0, 32);
		strcpy(ip_eth0, "cannot get");
	}

	if (get_ip("wlan0", ip_wlan0) != 0)
	{
		memset(ip_wlan0, 0, 32);
		strcpy(ip_wlan0, "cannot get");
	}
	disp_ip(ip_eth0, ip_wlan0);
	return 0;
}

int update_time()
{
	static int sec_last = 0;
	static int sec_this = 0;
	PAINT_TIME paint_time;
	time_t timep;
	struct tm* p;
	
	time(&timep);
	p = localtime(&timep);
	paint_time.Year = p->tm_year + 1900;
	paint_time.Month = p->tm_mon + 1;
	paint_time.Day = p->tm_mday;
	paint_time.Hour = p->tm_hour;
	paint_time.Min = p-> tm_min;
	paint_time.Sec = p->tm_sec;
	
	sec_this = p->tm_sec;
	if (sec_this != sec_last)
	{
		sec_last = sec_this;
		disp_time(&paint_time);
	}
	return sec_this;
}

static void cfinish(int sig)
{
	signal(SIGINT, NULL);
	mqtt_listen_stop = 1;
}


int main()
{
	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);

	int rv = mqtt_sub_start();
	printf("subscribe stop:%d\n", rv);
	return 0;
}

