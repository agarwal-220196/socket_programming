/*********************************TFTP server implementation*************/
//Author: Sanket Agarwal and Dhiraj Kudva
//Organisation: Texas A&M university
//Usage: used to html_request the html page from the proxt server. 

// list of common files required for the functioning. 
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
#include <common_def.h>

/*#define PORT_NO 80 
#define RETRIES 10
#define CACHE_ENTRIES 10
#define NAME_LENGTH 256
#define TIME_LENGTH 50
#define FILE_NAME_LENGTH 10
#define QUEUE 50
#define FILE 512
#define BUFFER 10000*/


/*typedef struct {
	
	char u_r_l[NAME_LENGTH];
    char Last_Modfd[TIME_LENGTH];
    char Exp[TIME_LENGTH];
    char Fname[FILE_NAME_LENGTH];
    int  Is_filled;
	
}cache;


cache cache[CACHE_ENTRIES];

*/


int main(int argc, char*argv[])
{
	char buff[10000];
    unsigned int pt_no;
    struct sockaddr_in address_ip4, remote_address_ip4;
    int client_file_descriptor;
    char html_request[200];
    char *name_of_host;
    char *path;
    char *path_copy;
    char *path_counter;
    int count = 0, flag_end = 0;
   // FILE * ptr_file;
	
	if (argc<4)// input is not as per exepctation 
	{
		system_error("CLIENT: ./client <ip> <port> <url>");
	}
	
	pt_no = atoi(argv[2]); // get the port number in string to integer value. Same as previous assignments. 
    
	
	//setting the address of ip, same as previous mps. 
	
    bzero(&remote_address_ip4, sizeof(remote_address_ip4));
	
	remote_address_ip4.sin_family = AF_INET;
    remote_address_ip4.sin_port = htons(pt_no);// same as previous mps
    if(inet_aton(argv[1], (struct in_addr *)&remote_address_ip4.sin_addr.s_addr) == 0){
        system_error(argv[1]);
        
    }
    client_file_descriptor = socket(AF_INET, SOCK_STREAM, 0); // same as previous mps 
    if(client_file_descriptor < 0){
        system_error("Socket Creating Error:");
        
    }
	// trying to connect, same as last time. 
    if(connect(client_file_descriptor, (struct sockaddr *)(&remote_address_ip4), sizeof(struct sockaddr)) < 0){
        system_error("Connection Error:");
        
    }

// first clear the contents of the request
    memset(html_request, 0, 100);
	//printf("First Checkpoint\n");
	// as per the html entered by user. 
    name_of_host = strtok(argv[3],"/");// basically it returns the first occurance of the delimiter token. 
    path = strtok(NULL, "");// same functionality as above. 
	//printf("Before sprintf\n");
	// print 
    sprintf(html_request,"GET /%s HTTP/1.0\r\nHost:%s\r\n\r\n", path, name_of_host);
    //printf("After sprintf\n");
    path_counter = path;
	//iterate until you get to the end of file. 
    while(*path_counter != '\0'){
        if(*path_counter == '/'){
            count++;
        }
        path_counter++;
    }
    path_copy = path;
	//printf("After while loop\n");
	// proceed only if the file count is morse than zero. 
    if(count > 0){
      //  printf("in if loop of count\n");
        while(count >= 0){
            if(flag_end == 0){
                strtok(path_copy, "/");// same functionality as above. 
                flag_end = 1;
            }
            else
            {
                path_counter = strtok(NULL, "/");
            }
            count--;
        }
    }
    else{
      //  printf("Checkpoint\n");
        path_counter = path_copy;
    }


// send the html_request to the server, after getting connected. 
    if((send(client_file_descriptor, html_request, strlen(html_request), 0)) == -1){
        system_error("Send Error:");
        exit(-1);
    }

   
	

	// the final file get copied in the path counter and not path. 
    FILE *ptr_file ;
    ptr_file= fopen(path_counter, "w");
    int msg_length;
	
	//printf("Checkpoint line 157\n");
	// clear the contents first
    memset(buff,0,10000); 
    
	// check if the receive is succesfll or not. In case of less than 0 report error. 
	if((msg_length = recv(client_file_descriptor, buff, 10000,0)) <= 0){
      //  printf("Checkpoint 2 \n");
        //printf("Message length :%d\n",msg_length );
        //system_error("Receive Error:");
    }
	// if the error received is 404 that means that the html page is not reachable. need to report it. and delete the file. 
    else if((*(buff+9) == '4') && (*(buff+10) == '0') && (*(buff+11) == '4')){
        //printf("Checkpoint  3\n");
        printf("%s", buff);
        remove(path_counter); // remove is an inbuilt c library function that deletes the file so that it is no longer accessible. 
    }
    else
    {// the file received seems ok, return the first occurance of any terminators.
        //printf("Last else\n");
        char * tp = strstr(buff, "\r\n\r\n");
        fwrite(tp+4, 1, strlen(tp)-4, ptr_file);
        memset(buff, 0, 10000);// clear the contents before receiving the actual file and writing to the client end. 
        while((msg_length = recv(client_file_descriptor, buff, 10000,0)) > 0){// keep on writing until the reveive function returns zero. 
            fwrite(buff, 1, msg_length,ptr_file);
            memset(buff, 0, 10000);

        }
        printf("SUCCESS: Read from the server\n");
        printf("File received : %s\n",path_counter);
    }
    fclose(ptr_file);
    close(client_file_descriptor);
    return 0;
}




















