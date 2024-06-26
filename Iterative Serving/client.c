#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>


#define PORT 12345
#define TIMEOUT 300

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int valread=0;

void *timer(void* args)
{
	time_t tstart= time(NULL);
	
	while(time(NULL) - tstart < TIMEOUT)
	{
		usleep(100000);
	} 
	printf("Server Busy :( try Again Later\n");
	exit(0);

}

int main() {


	for(int i=0;i<10;i++) fork();

    pthread_t timer_thread;

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char message[] = "Hi";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    //printf("Connected to the server\n");
    
    send(sock, "Waiting...", 10, 0);
 
        
        pthread_create(&timer_thread, NULL, timer, NULL);
        valread = read(sock, buffer, 1024);
        pthread_cancel(timer_thread);
        
        buffer[valread] = '\0';
        printf("%sto Chat\n", buffer);
    
	
	int c = 1;
	
    while (1) {
        //printf("Enter message: ");
        //fgets(message, sizeof(message), stdin);
            
        if(c>1000)   break;  
                
        c++;
        
        //message[strcspn(message, "\n")] = 0;  // Remove newline character

        //if (strcmp(message, "exit") == 0) {
       //     break;
        //}

        send(sock, message, strlen(message), 0);
        //printf("MESSAGE SENT\n");
        
        //pthread_create(&timer_thread, NULL, timer, NULL);
        valread = read(sock, buffer, 1024);
        //pthread_cancel(&timer_thread);
        
        buffer[valread] = '\0';
        //printf("Received from server: %s\n", buffer);
        
        usleep(1000);
    }

	//printf("Enter Rating for Server on a Scale of 1(Poor)-5(Good) : ");
	char rating = '4';
	//scanf("%c", &rating);
	send(sock, &rating, 1, 0);
    printf("Closing connection\n");
    close(sock);
    return 0;
}
