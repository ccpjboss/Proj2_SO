all: server client
server: server.c
	gcc -Wall -o server server.c -lrt -pthread

client: client.c
	gcc -Wall -o client client.c -lrt -pthread