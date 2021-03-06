#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "app_mqtt.h"
#include "app_disp_ctrl.h"


struct disp_ctrl disp_dev;

void select_my_image()
{
	Paint_SelectImage(disp_dev.image);
}

void disp_my_image()
{
	EPD_2IN13_Display(disp_dev.image);
}

int init_disp()
{
    disp_dev.image = NULL;
    pthread_mutex_init(&disp_dev.lock, NULL);
	if (DEV_Module_Init() != 0)
	{
		printf("init dev fail\n");
		return -1;
	}
    UWORD image_size = ((EPD_2IN13_WIDTH % 8 == 0)?(EPD_2IN13_WIDTH / 8) : (EPD_2IN13_WIDTH / 8 + 1)) * EPD_2IN13_HEIGHT;
    disp_dev.image =  (UBYTE*)malloc(image_size);
    if (disp_dev.image  == NULL)
	{
		printf("malloc fail\n");
		return -1;
	}
	EPD_2IN13_Init(EPD_2IN13_FULL);
	EPD_2IN13_Clear();
	// usleep(500000);	
	Paint_NewImage(disp_dev.image, EPD_2IN13_WIDTH, EPD_2IN13_HEIGHT, 270, WHITE);
	Paint_SelectImage(disp_dev.image);
	Paint_Clear(WHITE);
	EPD_2IN13_Display(disp_dev.image);
	// usleep(2000000);
	return 0;
}


int disp(const char * str)
{
	int y = 0;
	char* p = str;
	char tmp[64];
	char* pt = tmp;
	GET_LOCK();
	Paint_Clear(WHITE);
	while(*p != '\0')
	{
		if (y >= 7)
		{
			y = 0;
			EPD_2IN13_Display(disp_dev.image);
			usleep(5000000);
			Paint_Clear(WHITE);
		}
		if (*p == '\n')
		{
			*pt = '\0';
			Paint_DrawString_CN_extra(1,1+16*y, tmp, &Font1212CN, BLACK, WHITE, &y);
			y = (y-1)/16;
			y += 1;
			pt = tmp;
			p++;
		}
		else
		{
			*pt = *p;
			p++;
			pt++;
		}
	}
	*pt = '\0';
	Paint_DrawString_CN_extra(1,1+16*y, tmp, &Font1212CN, BLACK, WHITE, &y);
	EPD_2IN13_Display(disp_dev.image);
	RELEASE_LOCK();
	return 0;
	
}




