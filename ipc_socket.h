#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define port 8080
#define host "127.0.0.1"
#define LISTENQ 100

int open_listenfd();

int open_clientfd();
