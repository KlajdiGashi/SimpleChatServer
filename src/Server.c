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

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Formating the code */
void str_overwrite_stdout() {

    printf("\r%s", "> ");
    fflush(stdout);

}

/* Trims the newline character */
void str_trim_lf (char* arr, int length) {

  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;

    }
  }

}

/* Prints Client address byte by byte */
void print_client_addr(struct sockaddr_in addr){

    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);

}

/* Add clients to queue */
void queue_add(client_t *cl){

	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);

}

/* Remove clients to queue */
void queue_remove(int uid){

	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);

}

/* Send message to all clients except sender */

void send_message(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);
	time_t currentTime;
	time(&currentTime);
	struct tm* timeInfo = localtime(&currentTime);
	char timestamp[20];
	strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M;%S", timeInfo);
	char buffer[strlen(s) + 2 + strlen(timestamp)];
	sprintf(buffer, "[%s]%s", timestamp, s);
	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, buffer, strlen(buffer)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);

}

/* send message to specific client */
void send_message_c(const char *s, int uid){

	time_t currentTime;
	time(&currentTime);
	struct tm* timeInfo = localtime(&currentTime);
	char timestamp[20];
	strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M;%S", timeInfo);
	char buffer[strlen(s) + 2 + strlen(timestamp)];
	sprintf(buffer, "[%s]%s", timestamp, s);
	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				if(write(clients[i]->sockfd, buffer, strlen(buffer)) < 0){
					perror("ERROR: write to descriptor failed");
					break;

				}
			}
		}
	}
}
/* send list of active clients, /list? */
void send_active_clients(int uid){

    char s[64];

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i){
        if (clients[i]) {
            sprintf(s, "<<[%d] %s\r\n", clients[i]->uid, clients[i]->name);
            send_message_c(s, uid);
        }
    }
    pthread_mutex_unlock(&clients_mutex);

}

/* find client uid by name */
void send_client_by_name(char *name, char *buff) {
	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(!strcmp(clients[i]->name, name)){
				send_message_c(buff, clients[i]->uid);
				break;
			}
		}
	}
}