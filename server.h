// #ifndef MY_HEADER_H
// # define MY_HEADER_H

 #include <arpa/inet.h>
 #include <time.h>

typedef struct {
    int socket;
    struct sockaddr_in addr;
    time_t timestamp;
} ClientInfo;

typedef struct {
    char serverName;
    int  totalClients;
    int num_lostClients;
int numClientsForDay  ;
float averageRating ;

}Server;


#define PORT 12345
#define MAX_QUEUE_SIZE 1000
#define TIMEOUT 300  // 5 minutes in seconds



// int handle_client(void *);



// void *drop_old_connections(void *);

// void startServer();


void addClientToQueue(int , struct sockaddr_in);

// void *iterative_client_handler(void *);

// void *forking_client_handler(void *);

// void *multithread_client_handler(void *);

void startMultiThreadServer();

void startForkingServer();

void startIterativeServer();

// #endif
