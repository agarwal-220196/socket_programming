/**************************common_def.h************************************/
//Author:Sanket Agarwal & Dhiraj Kudva (agarwal.220196, dhirajkudva)@tamu.edu
//Organisation: Texas A&M University
//Description: common file definition to support server. 

#ifndef common_def
#define common_def

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <stdint.h>


#define PORT_NO 80 
#define RETRIES 10
#define CACHE_ENTRIES 10
#define NAME_LENGTH 256
#define TIME_LENGTH 50
#define FILE_NAME_LENGTH 10
#define QUEUE 50
#define FILE 512
#define BUFFER 10000

int system_error(const char* error_string);
// cache definition
typedef struct {
	
	char u_r_l[NAME_LENGTH];
    char Last_Modfd[TIME_LENGTH];
    char Exp[TIME_LENGTH];
    char Fname[FILE_NAME_LENGTH];
    int  Is_filled;
	
}cache;


cache cache[MAX_CACHE_ENTRIES];
// day structure definition
char *day[7] ={"Sun", "Mon", "Tues", "Wed", "Thurs", "Fri", "Sat"};


// month structure definition
char *month[12] ={"Jan","Feb","Mar","April","May","June","July","Aug","Sep","Oct","Nov","Dec"};


// the following function will be used to create the socket and at the same time also check the ipv4 vs v6
int socket_create_and_check(bool ip_v4_check)
{
	int socket_file_descriptor = -1;
    if (ip_v4_check ==  true)
    {
        socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    }
    else
    {// same as previous mps. 
        socket_file_descriptor = socket(AF_INET6, SOCK_STREAM, 0);
    }
    if(socket_file_descriptor == -1)
    {
        system_error("Socket Creation has Failed");
        
    }
    else{
        printf("Socket has been created .\n");
    }
    return socket_file_descriptor;
}


void server_address_create(struct sockaddr_in *servr_addr, char * ip_addr, int port_no)
{
	// same as previous mps. 
    bzero(servr_addr, sizeof(*servr_addr));
    (*servr_addr).sin_family = AF_INET;
    (*servr_addr).sin_addr.s_addr = inet_addr(ip_addr);
    (*servr_addr).sin_port = htons(port_no);
}


// the following function will be binding. same as previous mps. 

void server_bind (int socket_file_descriptor, struct sockaddr_in servr_addr)
{
	int return_value = bind(socket_file_descriptor, (struct sockaddr *) &servr_addr, sizeof(servr_addr));
    if(return_value != 0)
    {
        system_error("Socket Bind has Failed");
        
    }
    else
    {
        printf("Socket has been binded .\n");
    }
}

// define the listening function below
void being_listen(int socket_file_descriptor)
{
	int return_value = listen(socket_file_descriptor, QUEUE);
    if(return_value != 0)
    {
        system_error("Listen error occured");
     
    }
    else
    {
        printf("Server Listen mode begins\n");
    }
}


int parse_read_request(char *request, char *host, int *port,char *u_r_l, char *name)
{
	// this defines the type
	char method[NAME_LENGTH];
	
	// this defines the protocol
    char protocol[NAME_LENGTH];
    
	// complete url except end
	char *uri;
	
	//bytes to be returned
    int  return_bytes;
    return_bytes = sscanf(request, "%s %s %s %s", method, name, protocol, u_r_l);

    if(strcmp(method, "GET")!=0)
        return -1;

    if(strcmp(protocol, "HTTP/1.0")!=0)
        return -1;

    uri = u_r_l;
    uri = uri + 5;
    strcpy (u_r_l,uri);
    strcpy (host,u_r_l);
    strcat (u_r_l, name);

    *port = 80;
    return return_bytes; 
	
}

int check_if_cache_entry_present(char *u_r_l)
{
	int counter = -1;
    int counter_for = 0;
    for (counter_for = 0 ; counter_for < 10 ; counter_for++)
    {
        if (!strcmp (cache[counter_for].u_r_l, u_r_l))// compare if the received url matches with the one present in the table. 
        {
            counter = counter_for;
            break;// if yes, return the index. 
        }
    }
    return counter;
	
}



int system_error(const char* error_string) //display and exit 
{
	error(error_string);
	exit(1);
}
#endif 