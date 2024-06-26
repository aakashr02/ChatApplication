#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include<sched.h>
#include <sys/syscall.h>

#define PORT 12345
#define MAX_QUEUE_SIZE 1000
#define TIMEOUT 300  // 5 minutes in seconds

#define MAX_SERVICE_THREAD 16

typedef struct {
    int socket;
    struct sockaddr_in addr;
    time_t timestamp;
} ClientInfo;

 pid_t pid ;

char serverName;
ClientInfo client_queue[MAX_QUEUE_SIZE];
int queue_size = 0, totalClients=0, num_lostClients = 0;
	int numClientsForDay = 0;
		float averageRating = 0.0;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t dataUpdate = PTHREAD_MUTEX_INITIALIZER;



int handle_client(void *arg) {


	printf("PID = %d\n", (int)getpid());

    ClientInfo client = *((ClientInfo *)arg);
    char buffer[1024];
    int bytes_read;

    printf("Connected by %s:%d\n", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));
    
    char fileName[10];
    sprintf(fileName, "logs%c.txt", serverName);
    
    FILE* fptr = fopen(fileName, "a+");
	
        time_t log = time(NULL);
        char logTime[20];
        
        sprintf(logTime, "%s", ctime(&log) );
        logTime[strlen(logTime)-1] = '\0';
        fprintf(fptr, "%s - Server %c - Connected - %s:%d\n",  logTime, serverName, inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port)) ;
        
                
    
	int flag=0;

    while ((bytes_read = read(client.socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        //printf("Received from %s:%d: %s\n", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port), buffer);
        
        
        if( flag==1) write(client.socket, buffer, bytes_read);
        else 
        {
        	flag=1;
        	char server_name[30];
        	sprintf(server_name, "Server %c waiting ", serverName); 
        	write(client.socket, server_name, strlen(server_name) );
        }
        
        
        
    }

	read(client.socket, buffer, sizeof(buffer) - 1);  
	
	int rating = (int)buffer[0]-(int)'0';

	pthread_mutex_lock(&dataUpdate);
	
	
	numClientsForDay++;
		totalClients++;
		
		printf("Clients handled for the Day : %d\n", numClientsForDay);
		printf("Total Clients handled : %d\n", totalClients);
		printf("Total Clients Approached: %d\n", num_lostClients+totalClients);
		
	 
		averageRating = (((averageRating* (totalClients-1)) + rating)/ totalClients);
	printf("Average Rating of Server : %.2f\n", averageRating);
        
		
		fptr = fopen("logs.txt", "a+");
	
        	log = time(NULL);
        	
        
        	sprintf(logTime, "%s", ctime(&log) );
        	logTime[strlen(logTime)-1] = '\0';
        	fprintf(fptr, "%s - Server %c - DayHandled:%d, DayLost:%d, TotalHandled:%d, TotalApproached:%d, AverageRating:%.2f\n",  logTime, serverName, numClientsForDay, num_lostClients, totalClients, num_lostClients+totalClients, averageRating) ;
    
    		
	
	
	
	
	pthread_mutex_unlock(&dataUpdate);
	

    close(client.socket);
    printf("Connection closed by %s:%d\n", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));
    
    sprintf(logTime, "%s", ctime(&log) );
    logTime[strlen(logTime)-1] = '\0';
    fprintf(fptr, "%s - Server %c- Connection Closed - %s:%d\n", logTime, serverName, inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));
    
    fclose(fptr);
    
    return 0; //(int)buffer[0]-(int)'0';
}

void *client_handler(void *arg) {


	int count=0;

    while (1) {
        pthread_mutex_lock(&queue_mutex);
        if (queue_size > 0 && count<5) {
        
        	count++;
        
            ClientInfo client = client_queue[0];
            memmove(client_queue, client_queue + 1, (queue_size - 1) * sizeof(ClientInfo));
            queue_size--;
            pthread_mutex_unlock(&queue_mutex);
            
	
	
	const int STACK_SIZE = 65536;
    char* stack = malloc(STACK_SIZE);
    unsigned long flags = 0;
	pid_t newPid;
	if(pid==getpid()) 
	{	
		newPid = clone(handle_client, stack + STACK_SIZE, flags | SIGCHLD| CLONE_VM, &client);
		
	
	}       
                        count--;
            
        } else {
            pthread_mutex_unlock(&queue_mutex);
        }
        usleep(100000); // Sleep for 100ms
    }
    return NULL;
}

void *drop_old_connections(void *arg) {

	

    while (1) {
        time_t current_time = time(NULL);
        pthread_mutex_lock(&queue_mutex);
        for (int i = 0; i < queue_size; i++) {
            if (current_time - client_queue[i].timestamp > TIMEOUT) {
                close(client_queue[i].socket);
                printf("Dropped connection from %s:%d due to timeout\n", inet_ntoa(client_queue[i].addr.sin_addr), ntohs(client_queue[i].addr.sin_port));
                memmove(client_queue + i, client_queue + i + 1, (queue_size - i - 1) * sizeof(ClientInfo));
                queue_size--;
                i--;
                
                            pthread_mutex_lock(&dataUpdate);
                num_lostClients++;
                printf("Number of Lost Clients : %d\n", num_lostClients);
                printf("Total Clients Approached: %d\n", num_lostClients+totalClients);
                
                char fileName[10];
    sprintf(fileName, "logs%c.txt", serverName);
    
                FILE* fptr = fopen(fileName, "a+");
	
        	time_t log = time(NULL);
        	char logTime[20];
        
        	sprintf(logTime, "%s", ctime(&log) );
        	logTime[strlen(logTime)-1] = '\0';
        	fprintf(fptr, "%s - Server %c - DayHandled:%d, DayLost:%d, TotalHandled:%d, TotalApproached:%d, AverageRating:%.2f\n",  logTime, serverName, numClientsForDay, num_lostClients, totalClients, num_lostClients+totalClients, averageRating) ;
    
    		fclose(fptr);
    		
    		            pthread_mutex_unlock(&dataUpdate);
		
                
                
            }
        }
        pthread_mutex_unlock(&queue_mutex);
        sleep(1);
    }
    return NULL;
}

int main() {


	pid = getpid();

    
    FILE* readName = fopen("name.txt", "r" );
    serverName = fgetc(readName);
    fclose(readName);
    readName = fopen("name.txt", "w" );
    fputc( (char)(serverName+1) ,readName);    
    fclose(readName);	
    
	int opt = 1;

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t handler_thread, drop_thread;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

	printf("------------SERVER %c-----------\n", serverName);
	
	
	
	time_t today; 
  time(&today); 
  printf("Date & Time : %s", ctime(&today)); 
  
  printf("Number of Lost Clients : 0\n");


    printf("Server listening on port %d\n", PORT);

    pthread_create(&handler_thread, NULL, client_handler, NULL);
    pthread_create(&drop_thread, NULL, drop_old_connections, NULL);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        pthread_mutex_lock(&queue_mutex);
        if (queue_size < MAX_QUEUE_SIZE) {
            ClientInfo client;
            client.socket = new_socket;
            client.addr = address;
            client.timestamp = time(NULL);
            client_queue[queue_size++] = client;
        } else {
            close(new_socket);
            printf("Connection from %s:%d dropped: queue full\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        }
        pthread_mutex_unlock(&queue_mutex);
    }

    close(server_fd);
    
    return 0;
}
