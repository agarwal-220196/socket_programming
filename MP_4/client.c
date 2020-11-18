/*********Client.c****************/
/*Authors: Sanket Agarwal and Dhiraj Kudva
Organization: Texas A&M University
Description: client code .*/

//including the library files. these are standard library files. 
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc,char *argv[])
{
	FILE *file_pointer;
	int socket_descriptor, n, port_request = 80;
	char host_name[64], request[100], path_name[256], buffer_holder[100000];
	unsigned int pt_no;
	char *p, *ptr;
	struct sockaddr_in server_address_client;

	if (argc != 4) {
		system_error ("USAGE: ./client <Server_IP_Address> <Port_Number> <URL>");
		exit(1);
	}

	pt_no = atoi(argv[2]);
	bzero(&server_address_client,sizeof server_address_client);
	server_address_client.sin_family=AF_INET;
	server_address_client.sin_port=htons(pt_no);
	if (inet_aton(argv[1], (struct in_addr *)&server_address_client.sin_addr.s_addr) < 0)
		system_error ("ERROR: inet_aton error");

	socket_descriptor=socket(AF_INET,SOCK_STREAM,0);
	if (socket_descriptor < 0)
		system_error ("ERROR: Socket Error");

	if (connect(socket_descriptor,(struct sockaddr *)&server_address_client,sizeof(server_address_client)) < 0)
		system_error ("ERROR: Connect Error");
	bzero(request, sizeof(request));
	sprintf(request, "GET %s HTTP/1.0\r\n", argv[3]);
	fprintf(stdout, "Request sent to proxy server: \n%s\n", request);
	if (send(socket_descriptor, request, strlen(request), 0) < 0) {
		system_error("CLIENT: Send");
		exit(2);
	}
	bzero(buffer_holder, sizeof(buffer_holder));
	parse_client(argv[3], host_name, &port_request, path_name);
	file_pointer=fopen(host_name, "w");
	fprintf(stdout, "Waiting for response.....\n");
	if (recv(socket_descriptor, buffer_holder, 100000, 0) < 0) {
		system_error("CLIENT: In receiving");
		fclose(file_pointer);
		close(socket_descriptor);
		return 1;
	}
	if((strstr(buffer_holder, "200")) != NULL)
		fprintf(stdout, "'200 OK' received. Saving to file: %s\n", host_name);
	else if ((strstr(buffer_holder, "400") != NULL))
		fprintf(stdout, "'400 Bad Request' received. Saving to file: %s\n", host_name);
	else if ((strstr(buffer_holder, "404") != NULL))
		fprintf(stdout, "'404 Page Not Found' received. Saving to file: %s\n", host_name);
	else if ((strstr(buffer_holder, "301") != NULL))
		fprintf(stdout, "'301 Page' received but moved to https. Saving to file: %s\n", host_name);
	ptr = strstr(buffer_holder, "\r\n\r\n");
	fwrite(ptr + 4, 1, strlen(ptr) - 4, file_pointer);
	fclose(file_pointer);
	close(socket_descriptor);

	return 0;
}
