all:	run execute clean

run:	server.c client.c
	gcc server.c -o server
	gcc client.c -o client 

execute:	server.c
	./server
	
clean:	server client
	rm server
	rm client 
