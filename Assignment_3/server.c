/***********************TFTP Server code****************/
//Author:Sanket Agarwal and Dhiraj Kudva
//Organisation: Texas A&M University
//Usage: This code is used to communicate with the TFTP client and send back the received files from the client. 

//BONUS FEATURE: WRQ is implemented. 


#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LENGTH_BUFFER 520
#define MAX_DATA_SIZE 	  512
#define TIMEOUT 	  1

int next_character = -1;// would be used to read character by character and comare when to terminate. As explained in the mannual. 

int check_timeout(int file_descriptor)//timeout mechanisim for checking lost packets. will be setting the timer value as 1 sec as described in the mannual page 10
{
	fd_set receive_set;
	struct timeval time_value;
	FD_ZERO(&receive_set);
	FD_SET(file_descriptor, &receive_set);

	time_value.tv_sec = TIMEOUT;
	time_value.tv_usec =0;
	return (select(file_descriptor+1, &receive_set, NULL, NULL, &time_value));

}
 

void sigchild_handler()
{
	int child_errno = errno;
	int pid;
	while ((pid=waitpid(-1,NULL,WNOHANG))>0)
	{
		printf("child pid: %d \n",pid);
	}
	errno = child_errno;
}

int main(int argc, char *argv[])
{
	int socket_server, server_ip_address;
	int port_number;
	int receive_bytes;
	struct sockaddr_in server_address, client_address;
	socklen_t size_of_client;
	struct sigaction sig_add;

	char receive_buffer[MAX_LENGTH_BUFFER];
	char string_max[INET_ADDRSTRLEN];
	char send_buffer[MAX_LENGTH_BUFFER];
	
	int child_handle;
	char sent_data[MAX_DATA_SIZE];

	if (argc!=3)
	{
		printf("Number of arguments are incoorect. PLease use ./server <IP> <portnumber>\n");
		return 1;
	}

	memset(&server_address, 0, sizeof(server_address)); //initially resetting every thing to zero. 
	memset(&client_address,0, sizeof(client_address));
	memset(receive_buffer,0, sizeof(receive_buffer));
		
	server_ip_address = atoi(argv[1]);
	port_number = atoi(argv[2]);

	//setting the server address now. 

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_number);


	if ((socket_server = socket(AF_INET,SOCK_DGRAM,0))<0)
	{
		printf("server was not created due to %s \n", strerror(errno));
		return 1;
	}
	
	if ((bind(socket_server, (struct sockaddr*)&server_address, sizeof(server_address)))<0)
	{
		printf("Binding process failed due to %s \n",strerror(errno));		
		close(socket_server);
		return 1;
	}
	
	printf("Waiting for clients to join...");

	while (1) 
	{
		size_of_client = sizeof (client_address);
		if ((receive_bytes = recvfrom(socket_server, receive_buffer, sizeof(receive_buffer),0,(struct sockaddr*)&client_address, &size_of_client))<0)
		{
			printf("unable to receive over socket due to %s \n",strerror(errno));
			return 1;	
		}
		
		inet_ntop(AF_INET,&client_address.sin_addr, string_max, INET_ADDRSTRLEN);
		printf("connection estabilished with client from %s \n",string_max);

		sig_add.sa_handler = sigchild_handler;//remove zombie processes.
		sigemptyset(&sig_add.sa_mask);
		sig_add.sa_flags = SA_RESTART;
		
		if (sigaction(SIGCHLD, &sig_add, NULL)==-1)
		{	
			perror("sigaction failure");
			exit(1);
		}

		if (!fork())
		{
			char file_name[] = "";
			FILE *file_pointer;
			int op_code;
			int mode_of_op;
			size_t filename_length;
			char temp_file_name[10];
			
			op_code = receive_buffer[1];
			strcpy(file_name, &receive_buffer[2]);
			filename_length = strlen (file_name);
			printf("file name is %s \n",file_name);

// GET the mode of the incoming file. 
			if (strcasecmp(&receive_buffer[filename_length+3], "netascii")==0)
				mode_of_op = 1;
			else if (strcasecmp(&receive_buffer[filename_length+3],"octet")==0)
				mode_of_op = 2;
			else
			{
				//mode not specified. 
				printf("mode not specified.\n");
				memset(send_buffer,0, sizeof(send_buffer));
				send_buffer[1] =5; //error 
				send_buffer[3] =4; //illegal operation

				stpcpy(&send_buffer[4],"MODE NOT SPECIFIED");
				filename_length = strlen("MODE NOT SPECIFIED");
				
				if (sendto(socket_server, send_buffer, filename_length+5,0,(struct sockaddr *)&client_address, size_of_client)<0)
				{
					printf("sending error due to %s\n",strerror(errno));
					close(socket_server);
					return 1;
				}
	
			}

			if((file_pointer = fopen(file_name,"r")) ==NULL)
			{
				printf("FILE UNAVAILABLE\n");
				memset(send_buffer,0,sizeof(send_buffer));
				send_buffer[1] =5;//error
				send_buffer[3] = 1; //file not found 
				strcpy(&send_buffer[4],"FILE NOT FOUND");
				filename_length = strlen("FILE NOT FOUND");
				if (sendto(socket_server, send_buffer, filename_length+5,0,(struct sockaddr *)&client_address, size_of_client)<0)
				{
					printf("sending error due to %s\n",strerror(errno));
					close(socket_server);
					return 1;
				}
			}

		}

	}
	


}

























