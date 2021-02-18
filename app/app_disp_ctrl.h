#ifndef __APP_DISP_CTRL_H_
#define __APP_DISP_CTRL_H_
#include "Fonts/fonts.h"
#include "GUI/GUI_Paint.h"
#include "e-Paper/EPD_2in13.h"

struct disp_ctrl
{
    pthread_mutex_t lock;
    UBYTE* image;
};

extern struct disp_ctrl disp_dev;

#define GET_LOCK()              do {pthread_mutex_lock(&disp_dev.lock); }while(0)
#define RELEASE_LOCK()          do {pthread_mutex_unlock(&disp_dev.lock); }while(0)

#define DISP_X       122
#define DISP_Y      250

int init_disp();
int disp(const char * str);
void select_my_image();
void disp_my_image();
#endif
