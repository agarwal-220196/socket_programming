/*********Client code****************/
/*Authors: Sanket Agarwal and Dhiraj Kudva
Organization: Texas A&M University
Description: Client code to access the proxy server. Takes in the url from the client.*/

//including the library files. these are standard library files. 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <common_def.h>

int main(int argc, char *argv[])
{
	int socket_descriptor, ascii_to_int, connection;
	char buffer_to_receive_data[100000] = {0};
	int send_to_proxy;
	int receive_from_proxy;
	unsigned int pt_no;
	char*p, *pointer; 
	char request[100];
	char parse_path[256] = {0};
	char host_name[64] = {0};
	int  intial_port = 80;
	
	if (argc !=4){
		system_error("./client <IP> <Port no> <URL>");
	}
	
	pt_no = atoi(argv[2]);
	
	struct sockaddr_in server_address;
	
	bzero(&server_address, sizeof server_address);//intialize to zero
	server_address.sin_family = AF_INET;//Ipv4
	server_address.sin_port = htons(pt_no);// handle the endianess
	ascii_to_int = inet_aton(argv[1], (struct in_addr *)&server_address.sin_addr.s_addr);
	if (ascii_to_int <=0)
	{
		system_error("Client Error: inet_aton_error");
	}
	
	socket_descriptor = socket(AF_INET, SOCK_STREAM,0);
	if (socket_descriptor<0)
	{
		system_error("Client Error: socket creation error.");
	}
	
	connection = connect(socket_descriptor, (struct sockaddr *)&server_address,sizeof(server_address));
	
	if (connection<0)
	{
		system_error("Client Error: socket connection error.");
	}
	
	memset(request,0,100); // clear to zero
	
	sprintf(request, "GET %s HTTP/1.0\r\n", argv[3]); // get the complete url. Assuming it will fit in 100 characters. 
	printf("Sending the request to Proxy server now: \n%s\n",request);
	
	send_to_proxy = send(socket_descriptor, request, strlen(request),0);
	
	if (send_to_proxy == -1)
	{
		system_error("Client Error: unable to send it to the proxy.");
	}
	
	memset(buffer_to_receive_data, 0 ,100000);// intialise to zero. 
	
	parser_client(argv[3],host_name,&intial_port,parse_path);// parser will be done inside the function. And since the char pointer is passed, actuall value will be modified over here. 
	
	FILE * file_pointer;
	file_pointer = fopen(host_name, "w");
	printf("Response awaited from the proxy server... \n");
	receive_from_proxy = recv(socket_descriptor, buffer_to_receive_data,100000,0);
	
	if (receive_from_proxy<=0)
	{
		fclose(file_pointer);
		close(socket_descriptor);
		system_error("Client: file received");
	}
	// checking for different types of errors. 
	if ((strstr(buffer_to_receive_data, "200"))!=NULL)// find a substring. 
		printf("received sucess and saved to %s \n",host_name);
	else if((strstr(buffer_to_receive_data, "400") != NULL))
		printf("Bad_request and saved to %s\n", hostname);
	else if ((strstr(buff, "404") != NULL))
		printf("404 request and saved to %s\n", hostname);
	
	pointer = strstr(buffer_to_receive_data, "\r\n\r\n");
	fwrite(pointer+4,1,strlen(pointer)-4, file_pointer);
	fclose(file_pointer);
	close(socket_descriptor);
	
	return 0;
	
}