#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

// Port used by dictioanry service socket
#define port 8080

// Socket open only for localhost
#define host "127.0.0.1"

// Queue size is currently set to 100
#define LISTENQ 100

// This creates a server socket, binds it on a port and starts listening
int open_listenfd();

// This creates a client connection with the socket
int open_clientfd();
