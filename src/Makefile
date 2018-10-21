
all:
ifneq (../bin,$(wildcard ../bin))
	-mkdir ../bin
endif

ifneq (../etc,$(wildcard ../etc))
        -mkdir ../etc
endif

ifneq (../lib,$(wildcard ../lib))
        -mkdir ../lib
endif

#find . -name "*.sh" | xargs chmod a+x 
#cp ./script/*.sh ../bin/
	make all -C ./common/
	make all -C ./proxy/
	make all -C ./worker/
	
clean:
	make clean -C ./common/
	make clean -C ./proxy/
	make clean -C ./worker/

