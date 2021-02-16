.PHONY:all


SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${SRC})) 
all:$(OBJ)

${DIR_BIN}/%.o:./%.c
	$(CC) $(CFLAGS) -c  $< -o $@
