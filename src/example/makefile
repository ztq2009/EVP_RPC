###################################################################
# Vertion: 1.0
# Author:  tyreezhang
# History: 
##################################################################

PATH_RAPIDXML = ../../third/rapidxml-1.13
PATH_LIB = ../../lib
PATH_COMM = ../common
PATH_BIN = ../..//bin


CC       := g++
CFLAGS   += -I./ -I$(PATH_RAPIDXML) -I$(PATH_COMM)
CFLAGS   += -g -O -DNDEBUG -fpic -Wall
CFLAGS   += -std=c++0x
TARGET   := example.so
OBJS     := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
LIBS     := -L$(PATH_LIB) -lcommon -lpthread -ldl

all : ${TARGET}

${TARGET} : $(OBJS)
	$(CC) -O -shared -lc -o $@ $(CFLAGS) $^ $(LIBS)
	@cp -f $(TARGET) $(PATH_BIN) 
	
%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@    
	
clean:
	rm -f $(OBJS) $(TARGET) $(PATH_BIN)/$(TARGET)
	
