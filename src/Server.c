#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 100 /* Number of clients*/
#define BUFFER_SZ 2048  /* Data */

static _Atomic unsigned int cli_count = 0; /* Client counter */
static int uid = 10;

/* Client structure */
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid; 
	char name[32]; /* Max length 32 */
} client_t;
