######################################
#
######################################
#source file
SOURCE:= $(wildcard *.c) $(wildcard *.cpp)
OBJS:= $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

#target you can change test to what you want
TARGET:= main

#compile and lib parameter
CC:= arm-fsl-linux-gnueabi-gcc
LIBS:=-L. -L./lib
LDFLAGS:=-lpthread -lmodbus -lmosquitto -lyaml -lcjson
DEFINES:=
INCLUDE:=-I. -I./include 
CFLAGS:=-g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:=$(CFLAGS) -DHAVE_CONFIG_H -Wunused-variable

.PHONY :everything objs clean veryclean rebuild

everything : $(TARGET)

all : $(TARGET)

objs : $(OBJS)

rebuild: veryclean everything

clean :
	rm -fr *.o

veryclean : clean
	rm -fr $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) 