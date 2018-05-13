all: clean server client

server: 
	gcc -Wall -fsanitize=address -o server server.c

client: 
	gcc -Wall -fsanitize=address -o client client.c

clean:
	rm -f server client