/****************************client.c***********************/
//Author: Sanket Agarwal and Dhiraj Kudva
//Organisation: Texas A&M University 
//Description: Client code of the Simple Broadcast Chat Server 

//header files required 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <stdbool.h>

//Simple Broadcast Chat server structures. 
#include "common_def.h"
time_t local_time;

//connection is made, time to join the chat room

void join_chat (int socket_descriptor, char * argv[]){

	struct simple_broadcast_chat_server_header header;
	struct simple_broadcast_chat_server_attribute attribute;
	struct simple_broadcast_chat_server_message message;
	
	int join_status = 0;

	header.version = '3'; // protocol version as defined in the mannual
	header.type    = '2'; //join request header

	attribute.type = 2;//sending the username 
	attribute.length = strlen(argv[3]) + 1; //length of username + null char
	strcpy(attribute.payload_data, argv[3]); // copy the username 
	
	message.header = header; //encapsulate
	message.attribute[0] = attribute;// just one attribute for joining.

	write(socket_descriptor,(void *)&message, sizeof(message));

	join_status = read_server_message(socket_descriptor);

	if (join_status==1)
		{system_error("username already present.");
		 close(socket_descriptor);}

}	

//read server message

int read_server_message(int socket_descriptor){

	struct simple_broadcast_chat_server_message server_message;
	int i;
	int return_status = 0;
	int number_of_bytes = 0;

// reading the bytes from the server. 
	number_of_bytes = read(socket_descriptor,(struct simple_broadcast_chat_server_message*)& server_message, sizeof(server_message));

	int size_of_payload = 0;//will be used to check the size of the payload received. 
	
	// forward_message

	//check the username, compare the actual length with the length specified in the header, check the header type with the attribute index. If all are correct print the message. 

	if (server_message.header.type==3){// forward message
	
		if((server_message.attribute[0].payload_data!=NULL||server_message.attribute[0].payload_data!='\0')
		 &&(server_message.attribute[1].payload_data!=NULL||server_message.attribute[1].payload_data!='\0')
		 &&(server_message.attribute[0].type==4)
		 &&(server_message.attribute[1].type==2)){
		//checking the size of the payload matches the payload length specified. 
			for(i=0; i<sizeof(server_message.attribute[0].payload_data);i++){
				if (server_message.attribute[0].payload_data[i]=='\0'){//end of string found 
					size_of_payload = i-1;
					break;
				}
			}//end for 

			if(size_of_payload==server_message.attribute[0].length){
				
				printf("The user %s says %s",server_message.attribute[1].payload_data,
					server_message.attribute[0].payload_data);
				local_time = time (NULL);
			
				printf("CLIENT timestamp:%s \n",asctime(localtime(&local_time)));

			}

			else system_error("length of payload mismatch at client \n");
		}//mismatch of the any data such as payload data type or username data type 
		else system_error("CLIENT: header type mismatch or null recevied\n");

		return_status = 0; //sucess/
	}//if (server_message.header.type)


	//if the server sends a NACK
	if(server_message.header.type ==5){
		if((server_message.attribute[0].payload_data!=NULL||server_message.attribute[0].payload_data!='\0')
		 &&(server_message.attribute[0].type==1)){// indicating the reason of failure
			printf("Client: failure to join, reason %s",server_message.attribute[0].payload_data);
		}
		return_status = 1;		
	}// if (server_message.header.type)

//offline message received from the server. 
	if (server_message.header.type==6){
		
		if ((server_message.attribute[0].payload_data!=NULL||server_message.attribute[0].payload_data!='\0')
		   &&server_message.attribute[0].type ==2){ //i.e.sending the username that left the chat 
		
			printf("%s user left the chat room\n",server_message.attribute[0].payload_data);
		}
	return_status =0;//successfully read. 
	}

//ACK with the client count 
	if (server_message.header.type==7){
		if ((server_message.attribute[0].payload_data!=NULL||server_message.attribute[0].payload_data!='\0')		     &&server_message.attribute[0].type ==4){
		   printf("The number of client and ACK msg is %s\n",server_message.attribute[0].payload_data);
		}
	return_status =0;
	}

//new chat participant has arrived. 
	if (server_message.header.type ==8){
		if ((server_message.attribute[0].payload_data!=NULL||server_message.attribute[0].payload_data!='\0')
		   &&server_message.attribute[0].type ==2){ //i.e.sending the username that joinded
			printf("New %s user joined \n",server_message.attribute[0].payload_data);
		}

	return_status = 0; 
	}

	if (server_message.header.type ==9)
	{
		if ((server_message.attribute[0].payload_data!=NULL||server_message.attribute[0].payload_data!='\0')
		   &&server_message.attribute[0].type ==2){ //i.e.sending the username that joinded
			printf("%s user is IDLE \n",server_message.attribute[0].payload_data);
		}

	return_status = 0; 
	}


 
	return return_status;
}



void send_to_server(int socket_descriptor, bool timeout)
{
	struct simple_broadcast_chat_server_header header;
	header.version = '3';
	header.type    = '4';//as defined in the manual.

	struct simple_broadcast_chat_server_message message;
	struct simple_broadcast_chat_server_attribute attribute;


	message.header = header;// copying the header to the message header.

	int number_of_bytes_read_from_user = 0;
	char temp_message_holder[512];
//	char *pointer = temp_message_holder;
	struct timeval wait_time_to_send;
	fd_set read_file_descriptor;
	if (timeout == true)
	{
		header.version = '9';
		attribute.type =4;//idle message 
		char idle_array[29]  = "I am IDLE please talk to me.";
		idle_array[29] = '\0';
		strcpy(attribute.payload_data, idle_array);
		message.attribute[0] = attribute;
		message.attribute[0].length = 28; // the length of the string above. 
	}
	else 
	{

	wait_time_to_send.tv_sec = 2;
	wait_time_to_send.tv_usec = 0;

	FD_ZERO(&read_file_descriptor); //clearing the read descriptor
	FD_SET(STDIN_FILENO, &read_file_descriptor);// set to read from the input 

	select(STDIN_FILENO+1, &read_file_descriptor, NULL, NULL, &wait_time_to_send);

	if(FD_ISSET(STDIN_FILENO, &read_file_descriptor)){
		number_of_bytes_read_from_user = read(STDIN_FILENO,temp_message_holder, sizeof(temp_message_holder));

		if (number_of_bytes_read_from_user >0)
			temp_message_holder[number_of_bytes_read_from_user] = '\0';
	
	
	attribute.type = 4;//message as specified in the mannual 
	strcpy(attribute.payload_data, temp_message_holder);// copy the message to payload
	message.attribute[0] = attribute;
	message.attribute[0].length = number_of_bytes_read_from_user -1 ; //excluding the extra read char
	}else {

		printf("CLIENT:Timeout occrued \n");
	}

	}
	write(socket_descriptor, (void *)&message, sizeof(message));

	
}



 

int main (int argc, char*argv[]){

	//int idle_count = 0;
	struct timeval idle_count;
	char *username, *IPaddress;
	int select_return_value =0;
	if (argc!=4)
	{
		printf("CLIENT:USAGE:./client <IP_address> <port_number> <user_name> \n");
		system_error("Please specify the right arguments as above");
	}

	 // Wwould work for both IPv4 AND IPv6

	char * p; // used to point to an array if not converted,Same as MP1
	//server add. 
	int port_number = strtol(argv[2],&p,10);

//	struct hostent* IP =  gethostbyname(argv[1]);//IP address

	struct sockaddr_in server_address;
	struct sockaddr_in6 server_address_6; // IPv6 address
	struct addrinfo check , *get_addr_info=NULL; // will be used to check ipv4 vs 6.
	int return_value_address_resolution;
	int inet_return;
	int socket_descriptor;

	IPaddress = argv[1];
	username = argv[3];

	memset(&check, '\0', sizeof check);

	check.ai_family = PF_UNSPEC;
	check.ai_flags = AI_NUMERICHOST;

// get the inforamtion about the address being ipv4 or ipv6
	return_value_address_resolution = getaddrinfo(IPaddress, NULL, &check, &get_addr_info);

	if (return_value_address_resolution)
	{
		system_error ("invalid address");
	}

	if (get_addr_info->ai_family == AF_INET)// ipv4 address 
	{
	 socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&server_address, sizeof(server_address)); // same as MP 1
	server_address.sin_family = AF_INET; //IPv4
	server_address.sin_port   = htons(port_number); // port number as MP1
	//memcpy(&server_address.sin_addr.s_addr, IP->h_addr, IP->h_length);
	inet_return = inet_pton(AF_INET, IPaddress, &(server_address.sin_addr));	
	//connec to the server/or we can say the chatroom. 
	int connect_status = connect(socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address));
	if (connect_status < 0)//error
		system_error("Error connecting to the server");
	
	printf("Connection made, trying to join");
	}

	else if(get_addr_info->ai_family == AF_INET6)
	{
	 socket_descriptor = socket(AF_INET6, SOCK_STREAM,0);
	bzero(&server_address_6, sizeof(server_address_6));
	server_address_6.sin6_family = AF_INET6;
	server_address_6.sin6_port = htons(port_number);
	inet_pton(AF_INET6, IPaddress,&server_address_6.sin6_addr);

	int connect_status6 = connect(socket_descriptor, (struct sockaddr*)&server_address_6, sizeof(server_address_6));

	if (connect_status6<0)
		system_error("Client:ipv6 cant connect");
	} 

	else 
	{
		system_error("Invalid address.");
	}

	freeaddrinfo(get_addr_info);

	//adding file descriptor to the select. 
	fd_set main_file_descriptor;
	fd_set read_file_descriptor;
	
	//clearing them and setting to zero. 
	FD_ZERO(&read_file_descriptor);
	FD_ZERO(&main_file_descriptor); 


	join_chat(socket_descriptor, argv);

	FD_SET(socket_descriptor, &main_file_descriptor);// to see any input on the socket line 
	FD_SET(STDIN_FILENO, &main_file_descriptor);// to see any input on the command line 


	while (1){

	read_file_descriptor = main_file_descriptor;

	idle_count.tv_sec =10;// waitin for 10 secs or the readfiledescriptor to wakeup 

	if ((select_return_value = select(socket_descriptor+1, &read_file_descriptor,NULL,NULL,&idle_count))==-1)
		system_error("CLIENT:SELECT error");

	if (select_return_value == 0)// idle time of 10 secs have passed. 
	{	
		send_to_server(socket_descriptor, 1);
	}

	if (FD_ISSET(socket_descriptor,&read_file_descriptor))//read from the socket
		read_server_message(socket_descriptor);

	if (FD_ISSET(STDIN_FILENO, &read_file_descriptor))//read from commadn line send to the server
		send_to_server(socket_descriptor, 0 );
	}

	

	printf("The user left, end of chat\n");
	printf("Closing client");
	return 0;

}
