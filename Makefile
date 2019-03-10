all: dict_service dict_client

dict_service: dictionary_server.c ipc_socket.c
	gcc -pthread -g -o dictionary_service dictionary_server.c ipc_socket.c

dict_client: dictionary_server.c ipc_socket.c 
	gcc -g -o dictionary dictionary_client.c ipc_socket.c

clean:
	rm dictionary dictionary_service
