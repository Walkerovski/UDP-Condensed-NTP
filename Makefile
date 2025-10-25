CPP=g++
CPPFLAGS=-Iincludes -Wall -Wextra -ggdb -std=c++23 
LDLIBS=-lcrypto
VPATH=src
.INTERMEDIATE: parser_server.o server.o parser_client.o client.o requests.o

all: server client

server: parser_server.o server.o requests.o
	$(CPP) $^ $(LDLIBS) -o $@

client: parser_client.o client.o requests.o
	$(CPP) $^ $(LDLIBS) -o $@

clean:
	rm -rf *~ server client

.PHONY : clean all