GCCFLAGS = -Wall -g

all:
	$(MAKE) -C Comms
	$(MAKE) -C Client
	$(MAKE) -C Server

.PHONY : clean
clean:
	$(MAKE) -C Comms clean 
	$(MAKE) -C Client clean
	$(MAKE) -C Server clean
