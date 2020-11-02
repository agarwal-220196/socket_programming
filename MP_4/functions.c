/*********Common fucntions code****************/
/*Authors: Sanket Agarwal and Dhiraj Kudva
Organization: Texas A&M University
Description: functions for parsing the URL and generating system error messages.*/

//including the library files. these are standard library files. 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ELEMENT_NOT_IN_CACHE -1
#define MAX_CACHE_ENTRY 10
#define MAX_LEN 1024
#define STALE 1

struct cache_content {
	char url[256];
	char last_modified[50];
	char access_date[50];
	char expires[50];
	char *body;
};

int system_error(const char* x);
void parse_client(char* url_address, char *hostname_addr, int *port_address, char *path_address);
void parser_server(const char* header, char* buffer, char* output_file);


void parser_server(const char* header, char* buffer, char* output_file)
{
	// printf("Inside parser server\n");
	char *start_string = strstr(buffer, header);
	if (!start_string)
		return;
	char *end_string = strstr(start_string, "\r\n");
	start_string += strlen(header);
	while(*start_string == ' ')
		++start_string;
	while(*(end_string - 1) == ' ')
		--end_string;
	strncpy(output_file, start_string, end_string - start_string);
	output_file[end_string - start_string] = '\0';
}

void parse_client(char* url_address, char *hostname_addr, int *port_address, char *path_address) {
	// printf("Inside parse client\n");
	char *token_no, *temp_host_holder, *temp_path_address, *tmp1_holder, *tmp2_holder;
	int counter = 0;
	char string_holder[16];

	if (strstr(url_address,"http") != NULL) {
		token_no = strtok(url_address, ":");
		tmp1_holder = token_no + 7;
	} else
		tmp1_holder = url_address;
	tmp2_holder = (char *)malloc(64 * sizeof(char));
	memcpy(tmp2_holder, tmp1_holder, 64);
	if(strstr(tmp1_holder, ":") != NULL) {
		temp_host_holder = strtok(tmp1_holder, ":");
		*port_address = atoi(tmp1_holder + strlen(temp_host_holder) + 1);
		sprintf(string_holder, "%d", *port_address);
		temp_path_address = tmp1_holder + strlen(temp_host_holder) + strlen(string_holder) + 1;
	} else {
		temp_host_holder = strtok(tmp1_holder, "/");
		*port_address = 80;
		temp_path_address = tmp2_holder + strlen(temp_host_holder);
	}
	if (strcmp(temp_path_address, "") == 0)
		strcpy(temp_path_address, "/");
	memcpy(hostname_addr, temp_host_holder, 64);
	memcpy(path_address, temp_path_address, 256);
}

int system_error(const char* x)    // Error display source code
{ 
	perror(x);
	fprintf(stderr, "\n");
	exit(1); 
}

