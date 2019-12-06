
# Blue X-ray Plover Makefile
# gcc -o bxr_plover main.c -lrabbitmq -L ~/bxr_plover/src/include -L /usr/local/lib -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

CC = gcc
CFLAGS = -W -Wall $(INC) `pkg-config --cflags --libs gtk+-3.0`
TARGET = plover_client
OBJECTS = plover_client.o utils.o iniparser.o dictionary.o encode.o decode.o aria.o

INC = -I ~/bxr_plover/client/inc
SRC = ~/bxr_plover/client/src
LIB = -L ~/bxr_plover/client/lib -lrabbitmq -export-dynamic
BIN = ~/bxr_plover/client/bin

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB) 

clean :
	rm *.o $(TARGET)

