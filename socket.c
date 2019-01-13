//wiersz numer 15
//watki z wieloma klientami
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <pthread.h>

typedef int bool;
#define true 1
#define false 0

#define BUFFER_SIZE 1024
#define	DATA "PAAno swieta, co Jasnej bronisz Czestochowy"
#define DATA_LEN 44
#define MY_ADDR 111
#define ROW_SIZE 50
#define ROW_NUMBER 5

static int port_n = 7500;
static int client_port_n = 7500;
int server_fd;
int client_fd;

char poem[ROW_SIZE][250];

void close_sockets(){
	printf("Closing sockets...\n");
	close(server_fd);
	close(client_fd);
	printf("All sockets closed.\n");
}

void handleRequest(int socket_fd){
	printf("Server: Connected new client\n");
	char data_buffer[BUFFER_SIZE] = {0};

		char client_request_A[10];
		printf("%s", "Trying to read A");
		if (read(socket_fd, client_request_A, 1) < 0){
			//printf("Cannot get data from %d", i);
			return;
		}
		printf(": A received");
		if (client_request_A[0] == 'A'){
			char data_buffer_A[10] = {'5'};
	    
			if (write(socket_fd, data_buffer_A, 1) < 0) {
			    perror("!!!Sending error!!!\n");
			    return;
		    	}
		}
		printf("%s", "Trying to read B");
		char client_request_B[10];
		if (read(socket_fd, client_request_B, 1) < 0){
			//printf("Cannot get data from %d", i);
			return;
		}
		printf(": B received");
		if (client_request_B[0] == 'B'){

	    		//tu wysylamy wers
			if (write(socket_fd, DATA, DATA_LEN) < 0) {
			    	perror("!!!Sending error!!!\n");
			    return;
		    	}
		}
    close(socket_fd);
}

void *startServer(void * data){
    int new_socket;
	struct sockaddr_in address;
	int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("!!!Socket error!!!\n");
        return NULL;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("!!!Set socket error!!!\n");
        return NULL;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_n++);
    
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("!!!Bind error!!!\n");
        return NULL;
    }
    
	printf("Server started...\n");
    printf("Waiting for connection...\n");
    printf("=========================\n");
    
    if (listen(server_fd, 10) < 0) {
		perror("!!!Listen error!!!\n");
		return NULL;
	}

	do {
		if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
			perror("!!!Accept error!!!\n");
			return NULL;
		}
		printf("%s", "connected");
		handleRequest(new_socket);

	} while (true);
	
}

bool isInBlacklist(int *blacklist, int addr, int size){
	for (int i = 0; i < size; i++) {
		if (blacklist[i] == addr) return true;
	}
	return false;
}

void addToBlacklist(int *blacklist, int addr, int *size){
	int local_size = *size;
	blacklist[local_size] = addr;
	local_size++;
	*size = local_size;
}

void decodeMessage(char *buffer) {
	
	int lenght = 0;

	while(buffer[lenght] != '\0'){
		lenght++;
	}

	
	char poem_str[lenght];
	memcpy(poem_str, &buffer[0], lenght);
	printf("Wiersz: %s \n", poem_str);
	
	
	strcpy(poem[ROW_NUMBER], poem_str);
}

void *client(void* args) {
	printf("Client thread started\n");
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(client_port_n++);
	
	struct timeval timeout;      
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
	
	printf("Creating client socket...\n");
	

	int blacklist[255];
	blacklist[0] = MY_ADDR;
	int blacklist_counter = 1;
do{
	//for (int i = 108; i < 160; i++) 
	for (int i = 14; i <= 16; i++){ //ZAKRES HOSTOW W PODSIECI
		if (isInBlacklist(blacklist, i, blacklist_counter)){
			continue;
		}
		if (i==112) { continue;}
		//Create socket
		client_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(client_fd == -1){
			printf("Could not create socket");
		}
		if (setsockopt (client_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) printf("setsockopt failed\n");
		
        
		//printf("\n***********************\nClient socket created\n");

		char addr[20];
		//sprintf(addr,"192.168.102.%d",i);
		sprintf(addr,"10.0.2.%d",i); //TRZY OKTETY PODSIECI
		inet_aton(addr, &(server.sin_addr));
		server.sin_addr.s_addr = inet_addr(addr);

		printf("Trying connect to %s\n", addr);
		
		//Connect to server
		if (connect(client_fd, (struct sockaddr*)&server, sizeof(server)) < 0 ){
			//printf("Connect failed to %s\n", addr);
			continue;
		}

///////////////////////////////////////////////////////////////////////////////////////////////////////

		//Wyslanie A
		char data_buffer[10] = {'A'};
    		printf("A:%s", data_buffer);

		if (write(client_fd, data_buffer, 1) < 0) {
		    perror("!!!Sending error!!!\n");
		    return;
	    	}
	
		char server_reply_A[10];
		if (read(client_fd, server_reply_A, 1) < 0){
			//printf("Cannot get data from %d", i);
			continue;
		}
		printf("Server reply:%d", server_reply_A[0]);


		if (server_reply_A[0]-48 == ROW_NUMBER){
			char data_buffer_B[1] = {'B'};
	    
			if (write(client_fd, data_buffer_B, 1) < 0) {
			    perror("!!!Sending error!!!\n");
			    return;
		    	}
		}
		
		char server_reply[BUFFER_SIZE];
		if (read(client_fd, server_reply, BUFFER_SIZE) < 0){
			//printf("Cannot get data from %d", i);
			continue;
		}
		
		printf("Retrievied data from %s: %s\n", addr, server_reply);

		decodeMessage(server_reply); 
		
		addToBlacklist(blacklist, i, &blacklist_counter);
		
		close(client_fd);
	}
	
	printf("\n\n**************\nWIERSZ jak dotad!\n");
	for (int i = 0; i < ROW_SIZE; i++){
		printf("%d: %s\n",i,poem[i]);
	}
	
} while(true);

}

int main() {
	atexit(close_sockets);
	for (int i = 0; i < ROW_SIZE; i++){
		poem[i][0] = 0;
	}
  
    int NUMBER_OF_SERVER_THREADS = 2;
    pthread_t serverThreads[NUMBER_OF_SERVER_THREADS];
    pthread_t clientThreads[NUMBER_OF_SERVER_THREADS];
    int i;
	
    printf("Tworzę wątki serwerowe\n");   
    for (i = 0; i < NUMBER_OF_SERVER_THREADS; i++) {
        printf("Tworzę wątek nr - %d\n", i);
        pthread_create(&serverThreads[i],NULL,startServer,NULL);
        printf("Stworzono wątek nr - %d\n", i);
    }


    printf("Tworzę wątki klienckie\n");   
    for (i = 0; i < NUMBER_OF_SERVER_THREADS; i++) {
        printf("Tworzę wątek klienta nr - %d\n", i);
        pthread_create(&clientThreads[i],NULL,client,NULL);
        printf("Stworzono wątek nr - %d\n", i);
    }


    printf("Czekam na zakończenie wątków");
    for(i = 0; i < NUMBER_OF_SERVER_THREADS; i++) {
         pthread_join(serverThreads[i], NULL);
	pthread_join(clientThreads[i], NULL);
    }

    

	exit(0);
	


    return 0;
}
