#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "app_mqtt.h"
#include "application.h"


static void cfinish(int sig)
{
	signal(SIGINT, NULL);
	signal(SIGTERM, NULL);
	mqtt_listen_stop = 1;
}


int main()
{
	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);
	init_disp();
	idle_issue();
	mqtt_sub_start();
	while(1)
	{
		issue_schedule();
	}
	return 0;
}

