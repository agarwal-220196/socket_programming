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
	FD_SET(fd, &receive_set);

	time.tv_sec = TIMEOUT;
	time.tv_usec =0;
	return (select(fd+1, &receive_set, NULL, NULL, &tv));

}
 

void sigchild_handler()
{

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

/*following variables will be used afer calling if(!fork()) */
	char file_name[] = "";
	FILE *file_pointer;
	int op_code;
	int mode_of_op;
	char temp_file_name[10];

}

























