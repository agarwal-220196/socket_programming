//***************************************server.C****************************
//Author        : Sanket Agarwal & Dhiraj Kudva (agarwal.220196, dhirajkudva)@tamu.edu
//Organization  : Texas A&M University, Machine Problem 1 for ECEN 602
/*Description   : Contains the definition of server that takes the port number from the command line
		  to bind, listen and accept different IPv4 connections to that port number. It
                  listens to different connections and echos back the same message recevied by the 
                  client*/ 

//including standard libraries & common_def.h
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "common_def.h"

int main(int argc, char* argv[])
{

	char read_message [MAX_MESSAGE_LENGTH];
	int socket_fd, accept_fd, bind_fd, listen_fd;
	int port_number;
	char *pointer_after_number;
	int read_status, write_status;
	time_t local_time;

//	printf("Server:./echos <port number>\n");

	long convert_string_to_int = strtol (argv[1], &pointer_after_number,10);	//Same as client.C

	if (argc !=2)
	{
		system_error("SERVER_USAGE: ./echos <port number>");
	}
	port_number = convert_string_to_int;

	int pid;// will be used to identify child and parent process. 
	
	struct sockaddr_in clientaddr;
	printf("Server has started\n");
	socket_fd = socket(AF_INET, SOCK_STREAM,0); // same as client
	if (socket_fd <0)
		system_error("ERROR: socket descriptor error");
	
	bzero(&clientaddr , sizeof(clientaddr)); // same as client, setting to 0

	clientaddr.sin_family = AF_INET;
	/* we don't want to bind the server socket to a specific IP.
	Basically during bind, we want to accept connections to all IPs*/
	clientaddr.sin_addr.s_addr = htons(INADDR_ANY);
	clientaddr.sin_port = htons (port_number);
	
	//Binding the server now 
	bind_fd = bind(socket_fd, (struct sockaddr *) &clientaddr ,
			 sizeof(clientaddr));
	if (bind_fd < 0)
		system_error("ERROR: Server bind error");

	//listeing to different IP address on the binded port number 
	listen_fd = listen(socket_fd, 10);//10 is the max backlog
	if(listen_fd < 0)
		system_error("ERROR: Server listen error");
	
	/*now the socket server setup is complete and it has started listening
	Let us accept the connections in a continuous loop and echo back*/

	while (1)
	{	//there is no peer socket so we use NULL as the second arg
		accept_fd = accept(socket_fd, (struct sockaddr*) NULL, NULL);
		
		//create a new thread for this connection 
		pid = fork();
		
		if (pid > 0)// this is a parent 
		{
			printf("Server: Parent process with %d pid\n", pid);
			//if SIGCHILD close the connection and continue 
			
		}
		else if (pid ==0) // this a child 
		{
			while (1) // continue listening 
			{
				printf("Server:this is a child process \n");
				bzero(read_message ,(int) MAX_MESSAGE_LENGTH);
				
				printf("Server: Waiting to hear message\n");
				read_status = readline(accept_fd, read_message,
						      (int)MAX_MESSAGE_LENGTH);
printf("server readstat:%d\n",read_status);
				if(read_status > 0)
				{	
					printf("Server: Read from client %s\n", read_message);
					local_time = time(NULL);	
					printf("time of day:%s\n",asctime(localtime(&local_time)));
				}	

				printf("Server:Echoing back to client\n");
				write_status = written(accept_fd, read_message,
						      strlen(read_message));
		
				if(write_status > 0)
					printf("Server:Written back to client\n");

			}
		}
		else 
		{//pid creation error 
			printf("ERROR:fork() creation error with errono %d", errno);
			break;
		}
	
	}

	
}





