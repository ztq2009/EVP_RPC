###################################################################
# Vertion: 1.0
# Author:  tyreezhang
# History: 
##################################################################

PATH_RAPIDXML = ../../third/rapidxml-1.13
PATH_SRC = ../common
PATH_LIB = ../../lib


CC       := g++
CFLAGS   += -I./ -I$(PATH_RAPIDXML)
CFLAGS   += -g -O -DNDEBUG -fpic -Wall
CFLAGS   += -std=c++0x
TARGET   := libcommon.a
OBJS     := $(patsubst %.cpp,%.o,$(wildcard $(PATH_SRC)/*.cpp))

all : ${TARGET}

${TARGET} : $(OBJS)
	@ar crs $@ $^
	@cp -f $(TARGET) $(PATH_LIB) 
	
%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@    
	
clean:
	rm -f $(OBJS) $(TARGET) $(PATH_LIB)/$(TARGET)
	
