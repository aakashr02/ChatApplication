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
#include <sched.h>
#include <sys/syscall.h>

#include "server.h"

//Log Status Codes
#define CONNECT_SUCCESS '1'
#define CLOSED_SUCCESS '2'
#define TIMEOUT_DROPPED '3'
#define QUEUE_FULL_DROPPED '4'


//Mutex Locks for Client Request Queue
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

//Mutex Lock to update server data (number of clients served, average rating,...)
pthread_mutex_t dataUpdate = PTHREAD_MUTEX_INITIALIZER;

//Mutex lock to update buffer that hold log data
pthread_mutex_t logUpdate = PTHREAD_MUTEX_INITIALIZER;


//Buffer to hold log data before flushing
char logs[2048];

//Server object
Server server;

//Queue to hold Client Connections before serving
int queue_size = 0;
ClientInfo client_queue[MAX_QUEUE_SIZE];


/*
DESCRIPTION - 
This method writes logs periodically from the log buffer to the log files. It is invoked by a separate thread
that runs in parallel.

PARAMETERS - None
*/
void *logger(void *arg)
{
    char fileName[15];
    sprintf(fileName, "Server_%c.txt", server.serverName);


    while(1)
    {
        pthread_mutex_lock(&logUpdate);

        if(strlen(logs)>0)
        {
            FILE *fptr = fopen(fileName, "a+");

            fprintf(fptr, "%s",logs);

            fclose(fptr);

            memset( logs, 0, sizeof(logs) );
        }
            
        
        pthread_mutex_unlock(&logUpdate);

        //Thread sleeps for 1s 
        usleep(1000000);
    }
                

}

/*
DESCRIPTION - 
This method sends and receives messages from a client using the write and read methods.

PARAMETERS - void pointer typecasted to point a ClientInfo object.  
*/
int handle_client(void *arg)
{

    char logline[50];
        time_t log = time(NULL);
    char logTime[20];
    // char fileName[10];

    ClientInfo client = *((ClientInfo *)arg);
    char buffer[1024];
    int bytes_read;

    printf("Connected by %s:%d\n", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));


    // sprintf(fileName, "Server_%c.txt", server.serverName);

    // FILE *fptr = fopen(fileName, "a+");



    sprintf(logTime, "%s", ctime(&log));
    logTime[strlen(logTime) - 1] = '\0';
    
    sprintf(logline, "%s %c %d %c\n", logTime, server.serverName, ntohs(client.addr.sin_port), CONNECT_SUCCESS);
    
    while(strlen(logs)>2000) usleep(100000);
    pthread_mutex_lock(&logUpdate);
    strcat(logs, logline);
    pthread_mutex_unlock(&logUpdate);


    
    int flag = 0;

    while ((bytes_read = read(client.socket, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        // printf("Received from %s:%d: %s\n", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port), buffer);

        if (flag == 1)
            write(client.socket, buffer, bytes_read);
        else
        {
            flag = 1;
            char server_name[50];
            sprintf(server_name, "Server %c waiting to chat with Client %d", server.serverName, ntohs(client.addr.sin_port));
            write(client.socket, server_name, strlen(server_name));
        }
    }

    read(client.socket, buffer, sizeof(buffer) - 1);
    // printf("SERVER RATING : %c\n", buffer[0]);

    close(client.socket);
    printf("Connection closed by %s:%d\n", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));

    sprintf(logTime, "%s", ctime(&log));
    logTime[strlen(logTime) - 1] = '\0';
    //fprintf(fptr, "%s - Server %c- Connection Closed - %s:%d\n", logTime, server.serverName, inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));

    sprintf(logline, "%s %c %d %c\n", logTime, server.serverName, ntohs(client.addr.sin_port), CLOSED_SUCCESS);
    
    while(strlen(logs)>2000) usleep(100000);
    pthread_mutex_lock(&logUpdate);
    strcat(logs, logline);
    pthread_mutex_unlock(&logUpdate);



    pthread_mutex_lock(&dataUpdate);
    server.numClientsForDay++;

    server.totalClients++;

    printf("Clients handled for the Day : %d\n", server.numClientsForDay);
    printf("Total Clients handled : %d\n", server.totalClients);
    printf("Total Clients Approached: %d\n", server.num_lostClients + server.totalClients);

    int rating = (int)buffer[0] - (int)'0';

    if (rating > 0 && rating <= 5)
    {

        server.averageRating = (((server.averageRating * (server.totalClients - 1)) + rating) / server.totalClients);
        printf("Average Rating of Server : %.2f\n", server.averageRating);
    }
    log = time(NULL);

    pthread_mutex_unlock(&dataUpdate);

    return 0;
}

/*
DESCRIPTION -
This method serves clients one by one, taking a ClientInfo object from the client queue and passing it to the handle_client() method that serves it.

PARAMETERS - None
*/
void *iterative_client_handler(void *arg)
{

    while (1)
    {
        pthread_mutex_lock(&queue_mutex);
        if (queue_size > 0)
        {
            ClientInfo client = client_queue[0];
            memmove(client_queue, client_queue + 1, (queue_size - 1) * sizeof(ClientInfo));
            queue_size--;
            pthread_mutex_unlock(&queue_mutex);

            handle_client(&client);
        }
        else
        {
            pthread_mutex_unlock(&queue_mutex);
        }
        usleep(100000); // Sleep for 100ms
    }
    return NULL;
}


/*
DESCRIPTION -
This method handles multiple clients at a time by creating a separate thread for every new client. The number of concurrent threads is based on the number of cores.

PARAMETERS - None
*/
void *multithread_client_handler(void *arg)
{

    

    while (1)
    {
        pthread_mutex_lock(&queue_mutex);
        if (queue_size > 0)
        {
            ClientInfo client = client_queue[0];
            memmove(client_queue, client_queue + 1, (queue_size - 1) * sizeof(ClientInfo));
            queue_size--;
            pthread_mutex_unlock(&queue_mutex);

            pthread_t client_thread;

            pthread_create(&client_thread, NULL, (void *)handle_client, &client);

            
        }
        else
        {
            pthread_mutex_unlock(&queue_mutex);
        }
        usleep(100000); // Sleep for 100ms
    }
    return NULL;
}

/*
DESCRIPTION -
This method handles multiple clients at a time by creating a separate process for every new client. The number of concurrent processes is based on the number of cores.

PARAMETERS - void* typecasted to int* that holds the ID of the parent process
*/
void *forking_client_handler(void *arg)
{

    int pid = *((int *)arg);


    while (1)
    {
        pthread_mutex_lock(&queue_mutex);
        if (queue_size > 0 )
        {

    

            ClientInfo client = client_queue[0];
            memmove(client_queue, client_queue + 1, (queue_size - 1) * sizeof(ClientInfo));
            queue_size--;
            pthread_mutex_unlock(&queue_mutex);

            const int STACK_SIZE = 65536;
            char *stack = malloc(STACK_SIZE);
            unsigned long flags = 0;
            if (pid == getpid())
            {
                pid_t newPid = clone(handle_client, stack + STACK_SIZE, flags | SIGCHLD | CLONE_VM, &client);
            }
        
        }
        else
        {
            pthread_mutex_unlock(&queue_mutex);
        }
        usleep(100000); // Sleep for 100ms
    }
    return NULL;
}


/*
DESCRIPTION -
This method is run by a separate thread parallely to drop timed out clients from the clients_queue.

PARAMETERS - None
*/
void *drop_old_connections(void *arg)
{


    while (1)
    {
        time_t current_time = time(NULL);
        pthread_mutex_lock(&queue_mutex);
        for (int i = 0; i < queue_size; i++)
        {
            if (current_time - client_queue[i].timestamp > TIMEOUT)
            {
                
                close(client_queue[i].socket);
                time_t log = time(NULL);
                
                char logTime[20];

                sprintf(logTime, "%s", ctime(&log));
                logTime[strlen(logTime) - 1] = '\0';
                char logline[50];
                sprintf(logline, "%s %c %d %c\n", logTime, server.serverName, ntohs(client_queue[i].addr.sin_port), TIMEOUT_DROPPED);
    
                while(strlen(logs)>2000) usleep(100000);
                pthread_mutex_lock(&logUpdate);
                strcat(logs, logline);
                pthread_mutex_unlock(&logUpdate);

                printf("Dropped connection from %s:%d due to timeout\n", inet_ntoa(client_queue[i].addr.sin_addr), ntohs(client_queue[i].addr.sin_port));
                memmove(client_queue + i, client_queue + i + 1, (queue_size - i - 1) * sizeof(ClientInfo));
                queue_size--;
                i--;

                pthread_mutex_lock(&dataUpdate);
                server.num_lostClients++;
                printf("Number of Lost Clients : %d\n", server.num_lostClients);
                printf("Total Clients Approached: %d\n", server.num_lostClients + server.totalClients);

                pthread_mutex_unlock(&dataUpdate);
            }
        }
        pthread_mutex_unlock(&queue_mutex);
        sleep(1);
    }
    return NULL;
}

/*
DESCRIPTION -
This method initializes the struct Server object and assigns a name for the Server

PARAMETERS - None
*/
void startServer()
{
    FILE *readName = fopen("name.txt", "r");
    server.serverName = fgetc(readName);
    fclose(readName);
    readName = fopen("name.txt", "w");
    fputc((char)(server.serverName + 1), readName);
    fclose(readName);


    printf("------------SERVER %c-----------\n", server.serverName);
    time_t today;
    time(&today);
    printf("Date & Time : %s", ctime(&today));

    printf("Number of Lost Clients : 0\n");

    server.averageRating = 0.0;
    server.num_lostClients = 0;
    server.numClientsForDay = 0;
    server.totalClients = 0;
}

/*
DESCRIPTION -
This method adds incoming client connections to the client_queue before it is served by the server. 
Otherwise closes the client connection if the client_queue is full

PARAMETERS -
int new_socket - A number representing the new socket between client and server
struct sockaddr_in address - sockaddr_in holding the details of the newly created socket

*/

void addClientToQueue(int new_socket, struct sockaddr_in address)
{

    pthread_mutex_lock(&queue_mutex);
    if (queue_size < MAX_QUEUE_SIZE)
    {
        ClientInfo client;
        client.socket = new_socket;
        client.addr = address;
        client.timestamp = time(NULL);
        client_queue[queue_size++] = client;
    }
    else
    {
        close(new_socket);
        printf("Connection from %s:%d dropped: queue full\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        char logline[50];
        char logTime[20];
        time_t log = time(NULL);
        sprintf(logTime, "%s", ctime(&log));
    logTime[strlen(logTime) - 1] = '\0';
        sprintf(logline, "%s %c %d %c\n", logTime, server.serverName, ntohs(address.sin_port), QUEUE_FULL_DROPPED);
    
                while(strlen(logs)>2000) usleep(100000);
                pthread_mutex_lock(&logUpdate);
                strcat(logs, logline);
                pthread_mutex_unlock(&logUpdate);

    }
    pthread_mutex_unlock(&queue_mutex);
}

/*
DESCRIPTION -
This method creates 3 threads - 1 thread to handle client requests using multithreading, 1 thread to drop timed out requests, 1 thread for logger 

PARAMETERS - None
*/
void startMultiThreadServer()
{
    startServer();
    pthread_t handler_thread, drop_thread, logger_thread;
    pthread_create(&handler_thread, NULL, multithread_client_handler, NULL);
    pthread_create(&drop_thread, NULL, drop_old_connections, NULL);
    pthread_create(&logger_thread, NULL, logger, NULL);
}

/*
DESCRIPTION -
This method creates 3 threads - 1 thread to handle client requests using forking, 1 thread to drop timed out requests, 1 thread for logger 

PARAMETERS - None
*/
void startForkingServer()
{
    startServer();
    int pid = (int)getpid();
    pthread_t handler_thread, drop_thread, logger_thread;

    pthread_create(&handler_thread, NULL, forking_client_handler, &pid);
    pthread_create(&drop_thread, NULL, drop_old_connections, NULL);
    pthread_create(&logger_thread, NULL, logger, NULL);
}

/*
DESCRIPTION -
This method creates 3 threads - 1 thread to handle client requests using sequentially, 1 thread to drop timed out requests, 1 thread for logger 

PARAMETERS - None
*/
void startIterativeServer()
{
    startServer();
    pthread_t handler_thread, drop_thread, logger_thread;

    pthread_create(&handler_thread, NULL, iterative_client_handler, NULL);
    pthread_create(&drop_thread, NULL, drop_old_connections, NULL);
    pthread_create(&logger_thread, NULL, logger, NULL);
}
