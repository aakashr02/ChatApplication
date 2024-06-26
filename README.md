# ChatApplication
Chat Application using Socket Programming in C

The chat application is designed in the following 3 approaches.

### Approach 1 : Iterative Serving
In this approach, the clients are served one after the other, as the requests arrive. The requests are drpped after a certain timeout if they are not handled. A separate thread keeps a check on the timeout for each of the clients in the queue and removes the client that have timed out. Another thread, picks clients from the queue in FIFO order and serves it.

### Approach 2 : MultiThreading
In this approach, multiple clients are served simultaneously - a new thread is spawn for every new client. However, there is a cap set on the number of threads created (here MAX_THREADS is set to 16 as there are 16 cores). The other clients wait in the queue and are removed if the timeout is reached.

### Approach 3 : Forking
In this approach, multiple clients are served simultaneously - a new child process is created for every new client. However, there is a cap set on the number of threads created (here MAX_THREADS is set to 16 as there are 16 cores). The other clients wait in the queue and are removed if the timeout is reached. 


## Archival and Logging Performance
The performance of the servers before and after logging is as follows - 

**Iterative Serving**

![image](https://github.com/aakashr02/ChatApplication/assets/87864552/956cc4ce-a7cf-4adb-8d8b-43df9729195b)


**MultiThreading**

![image](https://github.com/aakashr02/ChatApplication/assets/87864552/3203c0fe-4d2d-4889-b4b8-e3b6d9d19494)


**Forking**

![image](https://github.com/aakashr02/ChatApplication/assets/87864552/0ceac787-a7c8-4df0-8214-33f43e6a2ff4)


## Improvements to Disk IO
The IOWait Percentage of the CPU was reduced by changing the values for the VM_Dirty_Ratio from system default value of 20 to new value of 60 and VM_Dirty_Background_Ratio from system default value of 10 to new value of 20.

**Dirty Ratio** - permitted amount of dirty pages in memory after which the process will itself write it back to disk.
**Dirty Background Ratio** - permitted amount of dirty pages after which the 'flush' thread will write it to disk, so that the processes can continue executing.

![image](https://github.com/aakashr02/ChatApplication/assets/87864552/d8fbab58-b5a2-427c-806f-465ab749ab0f)


