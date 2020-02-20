#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048

int flag = 0;
int sockfd = 0;
char name[32];

void commandline() {
  printf("%s", ":");
  fflush(stdout);
}

void ReplaceNewLineWithNull (char* arr, int length) {  //::::::::::::::::::::::::::::::::::::::::::
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void endWholeProgram(int sig) {
    flag = 1;
}

void sendhandler() {
  	char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {};

  while(1) {
  	commandline();  // :::::::::::::::::::::::::::::::::::::::::::::::::::
    fgets(message, LENGTH, stdin);
    ReplaceNewLineWithNull(message, LENGTH);  // ::::::::::::::::::::::::::::::::::::::::::::::::::

    if (strcmp(message, "exit") == 0) {
			break;
    } else {
      sprintf(buffer, "%s", message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

	bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }
  endWholeProgram(2);  //::::::::::::::::::::::::::::::::::::::::::::::::
}

void recv_msg_handler() {
	char message[LENGTH] = {};
  	while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
    	if (receive > 0) {
      		printf("%s", message);
      		commandline(); // :::::::::::::::::::::::
    	} else if (receive == 0) {
			break;
    	} else {
			// -1
		}
		memset(message, 0, sizeof(message));
  }
}

int main(int argc, char **argv){
	if(argc != 3){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = argv[2];
	int port = atoi(argv[1]);
	printf("%s",ip);
	signal(SIGINT, endWholeProgram);

	printf("Please enter your name: ");
  	fgets(name, 32, stdin);
  	ReplaceNewLineWithNull(name, strlen(name));     // ::::::::::::::::::::::::::::::::::::::::::::


	if (strlen(name) > 32 || strlen(name) < 2){
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  	server_addr.sin_family = AF_INET;
  	server_addr.sin_addr.s_addr = inet_addr(ip);
  	server_addr.sin_port = htons(port);

	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  	if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}
	send(sockfd, name, 32, 0);

	printf("=== Welcome to Virtual File System Via As client ===\n");

	pthread_t send_msg_thread;
  	if(pthread_create(&send_msg_thread, NULL, (void *) sendhandler, NULL) != 0)
	{
		printf("ERROR: pthread\n");
    	return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
  	if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){
			printf("\nBye\n");
			break;
    }
	}

	close(sockfd);

	return EXIT_SUCCESS;
}
