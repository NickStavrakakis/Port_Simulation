OUT = myport
OBJS = $(OUT) vessel port-master monitor
SOURCE = myport.c vessel.c port-master.c monitor.c port-master-functions.c global-functions.c
HEADER = header.h  port-master-functions.h global-functions.h
CC = gcc
FLAGS= -lpthread -o
CLEAR = clear_screen

all: $(CLEAR) $(OBJS)

clear_screen:
	clear

$(OUT): myport.c
	$(CC) myport.c $(FLAGS) $(OUT)

vessel: vessel.c
	$(CC) vessel.c global-functions.c $(FLAGS) vessel

port-master: port-master.c
	$(CC) port-master.c port-master-functions.c global-functions.c $(FLAGS) port-master

monitor: monitor.c
	$(CC) monitor.c global-functions.c $(FLAGS) monitor

clean:
	rm -f $(OBJS) $(OUT)
	rm -f *.o
	rm -f *.gch
	rm -f log.txt
	rm -f history.txt
