/*********************************TFTP server implementation*************/
//Author: Sanket Agarwal and Dhiraj Kudva
//Organisation: Texas A&M university
//Usage: Used to handle the TFTP requests from the client end. 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define MAX_SIZE 512

int system_error(const char *x)
{
	perror(x);
	exit(1);
}

int timeout(int file_descriptor, int time){
	
	fd_set receive_set;
	struct timeval time_value;
	FD_ZERO(&receive_set);
	FD_SET (file_descriptor, &receive_set);
	time_value.tv_sec = time;
	time_value.tv_usec = 0;
	
	return (select (file_descriptor + 1, &receive_set, NULL, NULL, &time_value)); //timeout will be called with 1 sec as described in the pdf. 

	
}

int main (int argc, char *argv[]){
	
	char data_read[512] = {0}; //initialising it to zero.
	int socket_file_descriptor, new_socket_file_descriptor;
	struct sockaddr_in client;
	socklen_t lenght_client = sizeof(struct sockaddr_in);
	
	int g, return_get_addr;
	int send_response;
	int last;
	
	int return_value, receive_byte;
	char buffer[1024] = {0};
	char ack[32] = {0};
	char payload[516] = {0};
	char payload_copy[516] = {0};
	char file_name[MAX_SIZE];
	char mode[512];
	
	unsigned short int op_code1, op_code2, block_number;
	int b,j;//general variables used inside for loops. 
	
	FILE *file_pointer;
	
	struct addrinfo address_dynamic, *addi, *client_info, *p;
	int true_yes;
	int pid;
	int read_return;
	int block_num=0;
	char ip[INET6_ADDRSTRLEN];
	int timeout_c = 0;
	int count =0;
	char c;
	char next_character = -1;
	int neg_ack = 0;
	char *ephemeral_pt;
	
	ephemeral_pt = malloc (sizeof ephemeral_pt);
	
	if (argc!=3)
	{
		system_error ("Incorrect USage: ./server <ip> <portnumber>");
		return 0;
	}
	
	
	
}