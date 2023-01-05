/*FTP Client*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#define MAX_LENGTH 1024

/*for getting file size using stat()*/
#include <sys/stat.h>

/*for sendfile()*/
#include <sys/sendfile.h>
#include <sys/wait.h>

/*for O_RDONLY*/
#include <fcntl.h>

static int COMMAND_SIZE = 15;
static int RESPONSE_SIZE = 1000;
static int clientSocket;
static int clientSocket1;
static int defaultSocket;
static int dataSocket;
static char dataPort[10];
static char loogedInUser[20];
static int pid;
static char *fileToUpload;
int position=0;
/**
1. socket call to get an a descriptor for client socket
2. connect to the client socket
3. return socket
**/

int createClientSocket(char serverPort[10])
{
    struct sockaddr_in server;
    int clientSocket, k;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        perror("socket creation failed");
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_port = atoi(serverPort);
    server.sin_addr.s_addr = 0;
    k = connect(clientSocket, (struct sockaddr *)&server, sizeof(server));
    if (k == -1)
    {
        perror("Connect Error");
        exit(1);
    }
    return clientSocket;
}
void abort_socket(int signal)
{
    close(dataSocket);
    exit(0);
}

/**
Handler for gracefully closing the client socket
**/
void CTR_handler()
{
    printf("\nsignal handler called :: closing client socket\n");
    close(clientSocket);
    exit(0);
}
/*
Sends the command to the server
*uses the send system call 
*/
void sendCommand(int clientSocket, char command[COMMAND_SIZE])
{
    send(clientSocket, command, COMMAND_SIZE, 0);
}

/**
Retrieves the command response from the server.
*uses the recv system call
**/
void receiveCommandResponse(int clientSocket, int isActiveCommand)
{
    char response[RESPONSE_SIZE];
    recv(clientSocket, response, RESPONSE_SIZE, 0);
    if (isActiveCommand)
    {
        // check if server is still active or not
        if (!strstr(response, "200 Server is active"))
        {
            printf("\nServer connection got closed :: disconnecting the client\n");
            close(clientSocket);
            exit(0);
        }
    }
    printf("\nServer: %s", response);
}
int main(int argc, char *argv[])
{

    int choice;
    signal(SIGINT, CTR_handler);
    signal(SIGSTOP, CTR_handler);

    if(argc > 1)
	{
        clientSocket = createClientSocket(argv[1]);
        
        defaultSocket = clientSocket;
        int i = 1;
		char command[500];
        char command2[500];
        read(clientSocket, command2, 500);
        printf("Received.. %s",command2);
        if(strcmp(command2, "next\n")==0){
            printf("Changed Server to Server 2..");
			clientSocket1 = createClientSocket(argv[2]);
            defaultSocket = clientSocket1;
			read(clientSocket1, command2, 500);
        }
        //dummy read for client2
        
        
        while (1)
		{
            
            memset(command2,"\0",500);
            printf("\n Enter Command : ");
            scanf("%s", command);
            if(strcmp(command,"quit")==0){
                close(clientSocket);
                close(clientSocket1);
                break;
            }
            strcat(command,"\n");
            int sent = write(defaultSocket, command, strlen(command));
            char newchar[500];
        
            read(defaultSocket, newchar, 500);
            printf("%s",newchar);
            memset(newchar,"\0",500);
			 
		}
	}
	return 0;
}