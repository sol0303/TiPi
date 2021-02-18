#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "app_mqtt.h"
#include "app_disp_ctrl.h"
#include "application.h"

#if 0



int disp_ip(const char* eth0, const char* wlan0)
{
	Paint_SelectImage(BlackImage);
	Paint_DrawString_EN(15 + 11*5, 15, eth0, &Font16, BLACK, WHITE);
	Paint_DrawString_EN(15 + 11*6, 35, wlan0, &Font16, BLACK, WHITE);
	EPD_2IN13_Display(BlackImage);
	return 0;
}





#endif 

static void cfinish(int sig)
{
	signal(SIGINT, NULL);
	mqtt_listen_stop = 1;
}


int main()
{
	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);
	init_disp();
	idle_issue();
	//disp("1.�����������¡�\n2.��������©�С�\n3.���Բ��еĹ���Ư����\n���л��������Ѷӳ���ǰ������!!");
	// mqtt_sub_start();
	return 0;
}

