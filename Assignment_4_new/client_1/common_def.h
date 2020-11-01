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
#define FILE_SIZE 512
#define BUFFER_SIZE 10000

int system_error(const char* error_string);
// cache definition
typedef struct {
	
	char u_r_l[NAME_LENGTH];
    char Last_Modfd[TIME_LENGTH];
    char Exp[TIME_LENGTH];
    char Fname[FILE_NAME_LENGTH];
    int  Is_filled;
	
}_cache;


_cache cache[CACHE_ENTRIES];
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
int check_if_cache_hit(char *u_r_l){
    int j=0;
    for(j=0;j<10;j++){
        if(strcmp(cache[j].u_r_l, u_r_l) == 0){
            return 0;
        }
    }
    return -1;
}
int connection_accept(struct sockaddr_in *client_address, int count_client, int socket_file_descriptor)
{
    socklen_t length =(socklen_t)sizeof(client_address[count_client]);
    int new_socket_filedescriptor = accept(socket_file_descriptor,(struct sockaddr*)&client_address[count_client], &length);
    if(new_socket_filedescriptor < 0){
        system_error("ERROR: IN ACCEPT CONNECTION");
        exit(-1);
    }
    return new_socket_filedescriptor;
}
void zombie_handler (int sigtemp_filedescriptor){
    wait(NULL);
}
int system_error(const char* error_string) //display and exit 
{
	error(error_string);
	exit(1);
}

int month_converter (char *month)
{
    int month_number;
    printf("In month_converter\n");
    switch(*month)
    {
        case 'J':
            if(*(month+1) == 'a')
            {
                month_number = 1;
            }
            else if (*(month+2) == 'n')
            {
                month_number = 6;
            }
            else
            {
                month_number = 7;
            }
            break;
        case 'F':
            month_number = 2;
            break;
        case 'M':
            if(*(month + 2) == 'r')
            {
                month_number = 3;
            }
            else
            {
                month_number = 5;
            }
            break;
        case 'A':
            if(*(month+2) == 'r')
            {
                month_number = 4;
            }
            else
            {
                month_number = 8;
            }
            break;
        case 'S':
            month_number = 9;
            break;
        case 'O':
            month_number = 10;
            break;
        case 'N':
            month_number = 11;
            break;
        case 'D':
            month_number = 12;
            break;
        default:
            break;
    }
    return month_number;
}

int check_if_cache_entry_expire(char *u_r_l, struct tm * time_now){
    printf("Second checkpoint\n");
    int i =0 , return_value;
    char updated_time[TIME_LENGTH];
    for(i=0;i<10;i++)
    {
        if(strcmp(cache[i].u_r_l,u_r_l)==0)
        {
            break;
        }
    }
    memset(updated_time,0, TIME_LENGTH);
    printf("Third checkpoint\n");
    sprintf(updated_time, "%s, %2d %s %4d %2d:%2d:%2d GMT", day[time_now->tm_wday],time_now->tm_mday, month[time_now->tm_mon], time_now->tm_year+1900,time_now->tm_hour,time_now->tm_min,time_now->tm_sec);
    printf("before sprintf\n");
    return_value = time_comparison(cache[i].Exp, updated_time);
    return (return_value<0)?-1:0;

}
int time_comparison(char *old_time, char *new_time){
    int year_old, month_old, hour_old,minute_old,second_old,day_old;
    int year_new, month_new, hour_new, minute_new, second_new, day_new;
    printf("In time comparison 0\n");
    char actual_old_month[4];
    char actual_new_month[4];

    memset(actual_new_month,0,4);
    memset(actual_old_month,0,4);

    sscanf(old_time + 5,"%d %3s %d %d:%d:%d ",&day_old, actual_old_month,&year_old,&hour_old,&minute_old,&second_old );
    sscanf(new_time + 5,"%d %3s %d %d:%d:%d ",&day_new, actual_new_month,&year_new,&hour_new,&minute_new,&second_new );
   
   printf("In time comparison\n");
    month_old = month_converter(actual_old_month);
    month_new = month_converter(actual_new_month);
    printf("In time comparison 2 \n");
    if(year_old < year_new) return -1;
    if(year_old>year_new) return 1;
    if(month_old<month_new) return -1;
    if(month_old > month_new) return 1;
    if (day_old < day_new) return -1;
    if(day_old > day_new) return 1;
    if(hour_old < hour_new) return -1;
    if(hour_old>hour_new) return 1;
    if(minute_old<minute_new) return -1;
    if(minute_old > minute_new) return 1;
    if (second_old < second_new) return -1;
    if(second_old > second_new) return 1;

    return 0;

}

void send_error_message (int status, int socket_file_descriptor){
    char errorMessage[1024];
    static char* bad_request = 
    "===================================================="
    "HTTP/1.0 400 Bad Request"
    "====================================================";

    static char *not_found = 
    "===================================================="
    "HTTP/1.0 404 not found"
    "====================================================";

    memset(errorMessage,0, 1024);
    switch(status)
    {
        case 400:
        sprintf(errorMessage,"%s", bad_request);
        send(socket_file_descriptor, errorMessage, strlen(errorMessage),0);
        break;

        case 404:
        sprintf(errorMessage,"%s",not_found);
        send(socket_file_descriptor, errorMessage, strlen(errorMessage),0);
        break;

        default:
        break;
    }
}
#endif 
