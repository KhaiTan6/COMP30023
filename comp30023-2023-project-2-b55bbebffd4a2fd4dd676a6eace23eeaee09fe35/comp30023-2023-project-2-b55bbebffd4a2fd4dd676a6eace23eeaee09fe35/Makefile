CC = cc
CFLAGS = -Wall -g
RPC_SYSTEM = rpc.o
# LDFLAGS = -L. 

.PHONY: format all

EXES = client server 


all:$(RPC_SYSTEM) 

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) $(CFLAGS) -c -o $@ $< 

server: 
	$(CC) $(CFLAGS) -o $@ server.a $(RPC_SYSTEM) 

client:
	$(CC) $(CFLAGS) -o $@ client.a $(RPC_SYSTEM) 

# RPC_SYSTEM_A=rpc.a
# $(RPC_SYSTEM_A): rpc.o
#	ar rcs $(RPC_SYSTEM_A) $(RPC_SYSTEM)

format:
	clang-format -style=file -i *.c *.h

clean:
	rm -f *.o $(EXES) 





