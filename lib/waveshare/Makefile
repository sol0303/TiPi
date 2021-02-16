PWD_DIR=$(shell pwd)
DIR_Config   = $(PWD_DIR)/Config
DIR_EPD      = $(PWD_DIR)/e-Paper
DIR_FONTS    = $(PWD_DIR)/Fonts
DIR_GUI      = $(PWD_DIR)/GUI

OBJ_C = $(wildcard ${DIR_EPD}/*.c ${DIR_GUI}/*.c  ${DIR_FONTS}/*.c)
OBJ_O = $(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))
RPI_DEV_C = $(wildcard $(DIR_BIN)/dev_hardware_SPI.o $(DIR_BIN)/RPI_sysfs_gpio.o $(DIR_BIN)/DEV_Config.o )


USELIB_RPI = USE_WIRINGPI_LIB
LIB_RPI=-Wl, -lwiringPi -lm 
DEBUG_RPI = -D $(USELIB_RPI) -D RPI

all:RPI_DEV ${OBJ_O} 

MSG = -g -O -Wall -I $(DIR_Config)
CFLAGS += $(MSG)


    
${DIR_BIN}/%.o:$(DIR_EPD)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config) $(DEBUG)
    
${DIR_BIN}/%.o:$(DIR_FONTS)/%.c 
	$(CC) $(CFLAGS) -c  $< -o $@ $(DEBUG)
    
${DIR_BIN}/%.o:$(DIR_GUI)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config) $(DEBUG)

${DIR_BIN}/%.o:$(DIR_APP)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config) $(DEBUG)


RPI_DEV:
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c  $(DIR_Config)/dev_hardware_SPI.c -o $(DIR_BIN)/dev_hardware_SPI.o $(LIB_RPI) $(DEBUG)
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c  $(DIR_Config)/RPI_sysfs_gpio.c -o $(DIR_BIN)/RPI_sysfs_gpio.o $(LIB_RPI) $(DEBUG)
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c  $(DIR_Config)/DEV_Config.c -o $(DIR_BIN)/DEV_Config.o $(LIB_RPI) $(DEBUG)
	

