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
				{mode_of_op = 1; printf("mode %d\n", mode_of_op);}
			else if (strcasecmp(&receive_buffer[filename_length+3],"octet")==0)
				{mode_of_op = 2;printf("mode %d \n", mode_of_op);}
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
			else
			{
				printf("file pointer %p\n",file_pointer);
			}

			//MODE REQUEST PROCESSING
			if(mode_of_op == 1)
			{
				fseek(file_pointer, 0, SEEK_END);
				int length_file = ftell(file_pointer);
				printf("LENGTH OF FILE = %d bytes\n",length_file );
				fseek(file_pointer, 0, SEEK_SET);

				FILE *temp_file_new;
				temp_file_new = fopen(temp_file_name, "w");

				int character = 1;

			//UNP REFERENCE ?

				while (character != EOF)
				{
					if(next_character >= 0)
					{
						fputc(next_character, temp_file_new);
						next_character = -1;
						continue;
					}

					character = fgetc(file_pointer);
					if(character == EOF)
					{
						if(ferror(file_pointer))
						{
							printf("ERROR in READ :: fgetc\n");
							break;
						}
					}
					else if (character == '\n')
					{
						character = '\r';
						next_character = '\n';
					}
					else if (character == '\r')
					{
						next_character = '\0';
					}
					else
						next_character = -1;
					fputc(character,temp_file_new);

				}
				fseek(temp_file_new,0,SEEK_SET);
				file_pointer = temp_file_new;
				file_pointer = fopen(temp_file_name,"r");

			}
			close(socket_server);

			//EPHEMERAL PORT : ANY PORT CAN BE USED BY BOTH SERVER AND CLIENT
			server_address.sin_port = htons(0);

			//socket creation for child provcess
			if((child_handle = socket(AF_INET, SOCK_DGRAM,0))<0)
			{
				printf("ERROR: CHILD SOCKET cound not be created\n");
				if(errno)
					printf("ERROR: EXITING due to : %d\n",strerror(errno) );
				return 1;
			}

			//binding socket to listen requests
			if((bind(child_handle,(struct sockaddr*)&server_address,sizeof(server_address)))<0)
			{
				printf("ERROR: UNABLE to bind the socket\n");
				if(errno)
					printf("ERROR: exiting due to error :%s\n",strerror(errno) );
				close(child_handle);
				return 1;
			}

			//Set of statements for sending data , ACK, indicating EOF and timeouts
			//Tracking packets

			unsigned short int number_sentblock, sentnumber;
			sentnumber=0;
			number_sentblock = 0;

			unsigned short int final_block, acknowledge_number;
			final_block = 0;
			acknowledge_number = 0;

			unsigned short int received_number;

			//track of offset
			unsigned int offset_file =0;
			int read_bytes;

			//send

			int sent_handler_bytes, recv_handler_bytes;
			int count_timeout = 0;
			int tv;

			while(1)
			{
				memset(send_buffer,0, sizeof(send_buffer));
				sprintf(send_buffer,"%c%c",0x00,0x03);

				sentnumber = number_sentblock + 1 ; //keeping track of the number of blocks and sends

				//writing the second and third entry in the send buffer to indicate this number
				send_buffer[2]= (sentnumber & 0xFF00) >> 8; //??
				send_buffer[3] = (sentnumber & 0x00FF);

				fseek(file_pointer,offset_file*MAX_DATA_SIZE, SEEK_SET);
				memset(sent_data,0,sizeof(sent_data));

				read_bytes = fread(sent_data,1, MAX_DATA_SIZE, file_pointer);

				//checking for the lastblock
				if(read_bytes < 512)
				{
					//indicates that it is the last block
					if(read_bytes == 0)
					{
						send_buffer[4]='\0'; //datablock in buffer
						printf("BLOCK of zero bytes\n");

					}
					else
					{
						memcpy(&send_buffer[4], sent_data,read_bytes);
						printf("Last block read: Size of block is less than 512 bytes\n");
					}
					final_block = number_sentblock;
					printf("Final Block = %d\n",final_block );
				}
				else
				{
					memcpy(&send_buffer[4],sent_data,read_bytes);
				}

				if((sent_handler_bytes = sendto(child_handle,send_buffer,read_bytes+4,0,(struct sockaddr* )&client_address,size_of_client)) < 0)
				{
					printf("ERROR: in sending\n");
					if(errno)
						printf("ERROR: Exiting due to error: %s\n",strerror(errno) );
					break;
				}

				//checking for timeouts
				if(tv = check_timeout(child_handle) == 0)
				{
					if(count_timeout == 10)
					{
						printf("MAXIMUM TIMEOUT REACHED = %d\n",count_timeout );
						break;
					}
					else
					{
						printf("ERROR: TIMEOUT OCCURED\n");
						count_timeout++;
						continue;
					}
				}
				else if(tv == -1)
				{
					printf("ERROR: In child select\n");
					if(errno)
						printf("ERROR: Exiting due to error : %s\n",strerror(errno) );
					break;
				}
				else
				{
					memset(receive_buffer,0,sizeof(receive_buffer));
					if((recv_handler_bytes = recvfrom(child_handle,receive_buffer,sizeof(receive_buffer),0,(struct sockaddr*)&client_address,&size_of_client))<0)
					{
						printf("ERROR: In Child receive\n");
						if(errno)
							printf("ERROR: Exiting error due to :%s\n", strerror(errno));
						break;
					}

					op_code = receive_buffer[1];
					memcpy(&received_number,&receive_buffer[2],2);
					acknowledge_number = ntohs(received_number);

					printf("OPCODE received from client: %d\n",op_code );
					printf("Ackowledgement received: %d\n", acknowledge_number );

					if(op_code == 4)
					{
						if(acknowledge_number == final_block && read_bytes < 512)
						{
							printf("Final Ackowledgement received\n");
							break;
						}
						number_sentblock ++ ;
						offset_file ++ ;

					}
					else
					{
						printf("Wrong acknowledge number format\n");
						break;
					}
				}


			}
			if(mode_of_op == 1)
			{
				if(remove(temp_file_name)!=0)
					printf("ERROR: In deleting the temp file\n");
				else
					printf("Success: Temp file is deleted\n");
			}
			close(child_handle);
			printf("Child is disconnected\n");
			exit(0);
		}

	}
	


}

























