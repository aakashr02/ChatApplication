# ChatApplication
Chat Application using Socket Programming in C

The chat application is designed in the following 3 approaches.

### Approach 1 : Iterative Serving
In this approach, the clients are served one after the other, as the requests arrive. The requests are dropped after a certain timeout if they are not handled. A separate thread keeps a check on the timeout for each of the clients in the queue and removes the client that have timed out. Another thread, picks clients from the queue in FIFO order and serves it.

### Approach 2 : MultiThreading
In this approach, multiple clients are served simultaneously - a new thread is spawn for every new client. The other wait in the queue and are removed if the timeout is reached.

### Approach 3 : Forking
In this approach, multiple clients are served simultaneously - a new child process is created for every new client. The clients wait in the queue and are removed if the timeout is reached. 


## Files and Description
**server.h** - Contains the declarations for the functions and data that are common to the 3 approaches.

**server.c** - Contains the implementation and logic for the function declarations in server.h

**iterative_server.c** - Contains the main method, socket creation for the server and invokes methods to handle multiple clients iteratively.

**forking_server.c** - Contains the main method, socket creation for the server and invokes methods to handle multiple clients using forking technique.

**multithreaded_server.c** - Contains the main method, socket creation for the server and invokes methods to handle multiple clients using multithreading.

**client.c** - Contains client side logic for socket creation and connection, sending and receiving messages.

**name.txt** - Holds the name of the server

**Server_A.log** - Sample log file

**Aggregate_JMeter_Results.xlsx** - Contains the load testing results conducted using Apache JMeter tool

## Performance before and after Logging (Disk IO)
The performance of the servers before and after logging is as follows - 

**Iterative Serving**

![image](https://github.com/aakashr02/ChatApplication/assets/87864552/9fc54b71-3fdd-4997-bf22-e08dab0f8f77)


**MultiThreading**

![image](https://github.com/aakashr02/ChatApplication/assets/87864552/55581462-0534-4f94-95d8-684161993a50)


**Forking**

![image](https://github.com/aakashr02/ChatApplication/assets/87864552/862b2390-0d36-47b1-911f-bed030e31717)


## Improvements to Disk IO

The following methods are used to improve disk IO through source code-
1. Buffering - The logs are stored in a buffer before they are written to the log file. This ensures that the logs remain in main memory and not every thread accesses the log file.
2. Periodic Logging - The logs in the buffer are logged only periodically, and this reduces the number of write operations done otherwise.
3. Multithreading - A separate thread apart from the worker threads is dedicated to write logs. The worker threads focus on serving the client.

The following methods are used to improve disk IO through kernel parameters-
The IOWait Percentage of the CPU was reduced by changing the values for the VM_Dirty_Ratio from system default value of 20 to new value of 60 and VM_Dirty_Background_Ratio from system default value of 10 to new value of 20.

**Dirty Ratio** - permitted amount of dirty pages in memory after which the process will itself write it back to disk.

**Dirty Background Ratio** - permitted amount of dirty pages after which the 'flush' thread will write it to disk, so that the processes can continue executing.

![image](https://github.com/aakashr02/ChatApplication/assets/87864552/d8fbab58-b5a2-427c-806f-465ab749ab0f)

There is not much difference in write_requests/s, KB_written/s, write_requests_merged/s and write_await parameters.

## Load Testing

Apache JMeter was used to load test the 3 approaches. The application was stressed with 900 Clients and 2 Servers with a ramp up time of 0.5s. Each client sent and received 1000 messages in 1s. The iterative-server appraoch gave a low throughput of 3 Requests/s while the forking and multithreading approaches gave a throughput of 18-19 Requests/s. For more detailed results refer the 'Aggregate_JMeter_Results.xlsx' file.

## Load Average 
The 1 minute CPU load average was ranging from 1 to 1.5 which indicates that the performance is good as the system has 16 CPUs and hence a load average upto 16 is acceptable.
