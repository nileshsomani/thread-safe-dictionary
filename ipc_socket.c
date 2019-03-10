#include "ipc_socket.h"

int open_listenfd() {
	int listen_fd, optval=1;
	struct sockaddr_in serveraddr;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	if (fcntl(listen_fd, F_SETFL, O_NONBLOCK) < 0)
		return -1;

	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0)
		return -1;

	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)port);
	if (bind(listen_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
		return -1;

	if (listen(listen_fd, LISTENQ) < 0)
		return -1;

	return listen_fd;
}

int open_clientfd() {
	int client_fd;
	struct hostent *hp;
	struct sockaddr_in serveraddr;

	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	if ((hp = gethostbyname(host)) == NULL)
		return -1;

	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
	serveraddr.sin_port = htons(port);

	if (connect(client_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
		return -1;

	return client_fd;
}

