PWD_DIR=$(shell pwd)
DIR_LIB_WAVESHARE = $(PWD_DIR)/lib/waveshare
DIR_LIB_PAHO = $(PWD_DIR)/lib/paho

DIR_APP = $(PWD_DIR)/app
DIR_BIN = $(PWD_DIR)/bin

DIR_INCLUDE = $(DIR_LIB_PAHO)
DIR_INCLUDE += -I $(DIR_LIB_WAVESHARE) 



##
CC = gcc
CFLAGS = -g -W -Wall -I $(DIR_INCLUDE)
# CFLAGS += -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -Werror=shadow -Werror=uninitialized -Werror=int-conversion
#CFLAGS += -Werror=implicit-function-declaration -Werror=uninitialized
LDFLAGS = -lpthread -lwiringPi -lm 

TARGET = tipi

export CC CFLAGS DIR_BIN DIR_INCLUDE


OBJ=$(wildcard ${DIR_BIN}/*.o)


##
all:objects
	$(CC) -o $(TARGET) $(OBJ) $(LIB) $(LDFLAGS)

objects:
	make -C $(DIR_LIB_WAVESHARE)
	make -C $(DIR_LIB_PAHO)
	make -C $(DIR_APP)

clean:
	rm -f ./bin/*.o
	rm -f ./$(TARGET)

