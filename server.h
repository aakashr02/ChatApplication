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





void addClientToQueue(int , struct sockaddr_in);

void startMultiThreadServer();

void startForkingServer();

void startIterativeServer();

