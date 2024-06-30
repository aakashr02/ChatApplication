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


int valread=0;

//function executed parallely by another thread to keep a check on timeout 
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


    pthread_t timer_thread;

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char message[50];

    //IPv4 TCP Socket Creation
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


    //Connecting to Server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }


    
    send(sock, "Waiting...", 10, 0);
 
        //Starting the Timer Thread
        pthread_create(&timer_thread, NULL, timer, NULL);
        valread = read(sock, buffer, 1024);

        //Timer Thread is killed after the first response is received
        pthread_cancel(timer_thread);
        
        buffer[valread] = '\0';
        printf("%s\n", buffer);
    
	
	int c = 1;
	
    while (1) {
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);
            
        // if(c>1000)   break;  
                
        // c++;
        
        message[strcspn(message, "\n")] = 0;  // Remove newline character

        if (strcmp(message, "exit") == 0) {
           break;
        }

        send(sock, message, strlen(message), 0);   

        valread = read(sock, buffer, 1024);
        
        buffer[valread] = '\0';
        printf("Received from server: %s\n", buffer);
        
        
        usleep(1000);
    }

    //Server Rating
	printf("Enter Rating for Server on a Scale of 1(Poor)-5(Good) : ");
	char rating;
	scanf("%c", &rating);
	send(sock, &rating, 1, 0);
    printf("Closing connection\n");

    //Closing the TCP socket
    close(sock);
    return 0;
}
