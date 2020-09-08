//***************************************client.C****************************
//Author        : Sanket Agarwal & Dhiraj Kudva (agarwal.220196, dhirajkudva)@tamu.edu
//Organization  : Texas A&M University, Machine Problem 1 for ECEN 602
/*Description   : Contains the definition of client socket creation, getting data from command line
		  passing the data to server and accepting the response. It takes the IPv4 addr and the 
		  socket number from command line along with the message and waits for the same message
                  to be echoed back to it from Server */ 

//including common libraries and common_def.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include "common_def.h"


int main (int argc, char*argv[])
//argc specifies the number of command line inputs and argv stores the input in array starting from 1
{

	int socket_descriptor, string_2_network_addr, connect_desc;
	char message_send[MAX_MESSAGE_LENGTH];
	char message_receive[MAX_MESSAGE_LENGTH + 1];//to receive the extra null character.
	int port_number;
	int read_status = 0;
	int write_status = 0;
	char *s;	//end character pointer. Points to the next character after the numerical value.
	struct timeval time_stamp_client;

	printf ("Client:  ./echo <IP_address> <port_number> \n");

	long string_to_long_int = strtol(argv[2], &s, 10); //(source, next str pointer, base)
	
	if (argc !=3){
		system_error("Please follow: ./echo <IP_addr> <Port_number> ");
		return 0;
	} 

	port_number = string_to_long_int;
	printf ("The port number is %d" , port_number);	

	struct sockaddr_in server_address;
	socket_descriptor = socket(AF_INET, SOCK_STREAM,0); //(IPv4, TCP, IP)

	if (socket_descriptor < 0)
		system_error("Error: Socket descripter error");

	bzero(&server_address, sizeof(server_address));	//clear the contents of the server_address.

	server_address.sin_family = AF_INET;//IPv4 family 
	server_address.sin_port	  = htons(port_number);//making sure it is in big endinan

	//converts the string number into a network address structure in the af adress family. 
	string_2_network_addr = inet_pton(AF_INET, argv[1], &(server_address.sin_addr));
	if (string_2_network_addr)
		system_error("network IP address in not correct");

	connect_desc = connect(socket_descriptor,(struct sockaddr *)&server_address, 
			       sizeof(server_address) );

	if (connect_desc < 0)
		system_error("Error in connecting");


//Now the connection is established and we can started sending the message in a continuous loop

	while (1)
	{
		bzero(message_send, MAX_MESSAGE_LENGTH);//refresh it to zero 
		bzero(message_receive, MAX_MESSAGE_LENGTH); //refresh the incoming messages to zero
		
		//Take message from command line and send it to the server. 
		fgets(message_send,(int)MAX_MESSAGE_LENGTH, stdin); // (destination, max_lenth, source)
		write_status = written(socket_descriptor, message_send, strlen(message_send));
		if (write_status > 0)
			printf("CLIENT: Message written to the server \n");
		else 
			printf("ERROR: CLIENT: Message not written to the server \n");

		//Receive the message from server and print it to the command line. 
		read_status = readline (socket_descriptor, message_receive,(int)MAX_MESSAGE_LENGTH);
		message_receive[read_status] = '\0';   	//end of string character
		if (read_status > 0)
		{
			printf("CLIENT: Message read from the server %s \n", message_receive);
			ioctl(socket_descriptor, SIOCGSTAMP, &time_stamp_client);
			printf("CLIENT timestamp: %d.%d \n", time_stamp_client.tv_sec, 
				time_stamp_client.tv_usec);
		}
		else
			printf("ERROR: CLIENT: Message not read from the server or blank message \n");
		read_status = 0; // get ready for next read
		
		
	}
	
	return 0; 
}
