/****************************client.c***********************/
//Author: Sanket Agarwal and Dhiraj Kudva
//Organisation: Texas A&M University 
//Description: Client code of the Simple Broadcast Chat Server 

//header files required 

#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sstream.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>


//Simple Broadcast Chat server structures. 
#include "common_def.h"


//connection is made, time to join the chat room

void join_chat (int socket_descriptor, char * arg[]){

	struct simple_broadcast_chat_server_header header;
	struct simple_broadcast_chat_server_attribute attribute;
	struct simple_broadcast_chat_server_message message;
	
	int join_status = 0;

	header.version = '3'; // protocol version as defined in the mannual
	header.type    = '2'; //join request header

	attribute.type = 2;//sending the username 
	attribute.length = strlen(arg[3]) + 1; //length of username + null char
	strcpy(attribute.payload_data, arg[3]); // copy the username 
	
	message.header = header; //encapsulate
	message.attribute[0] = attribute;// just one attribute for joining.

	write(socket_descriptor,(void *)&message, sizeof(message));

	join_status = read_server_message(socket_descriptor);

	if (join_status==1)
		system_error("username already present.");

}	
 

int main (int argc, char*argv[]){

	if (argc!=4)
	{
		printf("CLIENT:USAGE:./client <IP_address> <port_number> <user_name> \n");
		system_error("Please specify the right arguments as above");
	}
	
	int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0) // same as MP1

	//server add. 

	struct hostent* IP =  gethostbyname(arg[1]);//IP address
	struct sockaddr_in server_address;
	bzero(&server_address, sizeof(server_address)); // same as MP 1
	server_address.sin_family = AF_INET; //IPv4
	server_address.sin_port   = htons(atoi(argv[2])); // port number as MP1
	memcpy(&server_address.sin_addr.s_addr, IP->h_addr, IP->h_length);


	//adding file descriptor to the select. 
	fd_set main_file_descriptor;
	fd_set read_file_descriptor;
	
	//clearing them and setting to zero. 
	FD_ZERO(&read_file_descriptor);
	FD_ZERO(&main_file_descriptor); 

	//connec to the server/or we can say the chatroom. 
	int connect_status = connect(socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address));

	if (connect_status < 0)//error
		system_error("Error connecting to the server");

	printf("Connection made, trying to join");

	join_chat(socket_descriptor, argv);



	






}
