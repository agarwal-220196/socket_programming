#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <sstream.h>
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

#define max_string_size 256

int number_of_clients = 0;

//initializing clients structure to store client information
struct simple_broadcast_chat_server_client_user_information *clients;

//ACK function: To send acknowledge signal to client
void ACK_server_client (int fd_client){
	struct simple_broadcast_chat_server_message ack_message;
	char temp_string [256];

	//ACK header format
	ack_message.header.version = 3; //mandatory header type FWD
	ack_message.header.type = 7;
	ack_message.attribute[0].type = 4;

	snprintf(temp_string,5,"%d%s",number_of_clients," ");
	int k = strlen(temp_string);
	int i;
	for (i=0; i < number_of_clients-1; i++)
	{
		strcat(temp_string,",");
		strcat(temp_string,clients[i].username);
	}
	ack_message.attribute[0].length = strlen(temp_string) + 1;
	strcpy (ack_message.attribute[0].payload_data, temp_string);
	
	write (fd_client,(void *)&ack_message, sizeof(ack_message));
}

void NACK_server_client(int fd_client)
{
	struct simple_broadcast_chat_server_message nack_message;
	char temp_string[256];

	nack_message.header.version = 3;
	nack_message.header.type = 5;
	nack_message.attribute[0].type = 1;

	strcat(temp_string,"Client with same username already exists. Try with another username");
	nack_message.attribute[0].length = strlen(temp_string);
	strcpy(nack_message.attribute[0].payload_data, temp_string);

	write(fd_client,(void *) &nack_message,sizeof(nack_message));

	close(fd_client);
}

//checks clients username:
//if username exists, returns true
//else returns false
int check_username (char username[]){
	int i;
	for (i =0 ; i < number_of_clients; i++)
	{
		if(strcmp (username, clients[i].username) == 0)
		{
			return 1; //username exists
		}
	}
	return 0; //username doesnot exist
}

//checks if the client has joined or not
int client_join_check(int fd_client){
	struct simple_broadcast_chat_server_message join_message;
	struct simple_broadcast_chat_server_attribute join_message_attribute;
	char temp_string [16];
	read(fd_client,(struct simple_broadcast_chat_server_message *)&join_message,sizeof(join_message));
	join_message_attribute = join_message.attribute[0];
	strcpy(temp_string,join_message_attribute.payload_data);

	if(check_username(temp_string)){
		printf("%s\n","Client with this username already exists !" );
		NACK_server_client(fd_client);
		return 1;
	}
	else{
		strcpy(clients[number_of_clients].username, temp_string);
        printf("number_of_clients = %d\n",number_of_clients);
		clients[number_of_clients].file_descriptor = fd_client;
		clients[number_of_clients].client_count = number_of_clients;
		number_of_clients ++;
		ACK_server_client(fd_client);

	}
	return 0;
}


int main(int argc, char*argv[])
{
	struct simple_broadcast_chat_server_message receive_message, forward_message, join_broadcast_message, leave_broadcast_message;
	struct simple_broadcast_chat_server_attribute client_attribute;

    printf("Here it is\n");
	//Server's address information
	struct sockaddr_in server_address, *clients_address;
    server_address.sin_family = AF_INET;
    printf("First chec\n");
    // server_address.sin_addr.s_addr = inet_addr(INADDR_ANY);
    printf("Second checpoint \n");
    server_address.sin_port = htons(atoi(argv[1]));
    socklen_t server_addr_size = sizeof(server_address);

    int maximum_number_of_clients = atoi(argv[2]);
    printf("Maximum number%d\n", maximum_number_of_clients);

    int server_status;
    struct addrinfo addr_hints;
    struct addrinfo *server_info; //point to the results
    memset(&addr_hints,0,sizeof addr_hints); //creating an empty structure
 
    addr_hints.ai_family = AF_UNSPEC; //don;t care IPv4 OR IPv6
    addr_hints.ai_socktype = SOCK_STREAM; //TCP sockets

    // if((server_status = getaddrinfo(argv[1],&addr_hints, &server_info))!=0){
    // 	fprintf(stderr, "ERROR: getaddrinfo : %s\n",gai_strerror(server_status) );
    // 	exit(1);
    // }

    fd_set fd_master; //main master set of file descriptor
    fd_set temp_fd; //temporary file decriptor list
    FD_ZERO (&fd_master); //resetting all entries in the temp and master file descriptors
    FD_ZERO (&temp_fd);

    int client_new = 0; //count for newly accepted socket descriptor
    struct sockaddr_in addr_client; //client's address
    // addr_client.sin_addr.s_addr = htons(INADDR_ANY);
    addr_client.sin_family = AF_INET;
    /* we don't want to bind the server socket to a specific IP.
    Basically during bind, we want to accept connections to all IPs*/
    addr_client.sin_addr.s_addr = htons(INADDR_ANY);
    addr_client.sin_port = htons (atoi(argv[1]));
    int number_of_bytes = 0; //number of bytes received
    printf("Third checpoint\n");
    //socket initialization
    // int socket_server = socket((*server_info).ai_family,(*server_info).ai_socktype,(*server_info).ai_protocol);
    int socket_server = socket(AF_INET, SOCK_STREAM,0);
    printf("Socket server value %d\n",socket_server );
    if(socket_server < 0){
    	printf("%s\n","Failed to establish connection between client and server" );
    	system_error("Trying to establish");
    	exit (0);
    }

    int temp_reuse = 1;
    if (setsockopt(socket_server,SOL_SOCKET,SO_REUSEADDR,&temp_reuse,sizeof(int))<0){
    	printf("%s\n","Failed : setsockopt(SO_REUSEADDR)" );
    }
    //CHECK THIS CASE: WHEN should exactly the below message needs to be displayed

    printf("Rstablished socket for server");

    //binding
    // if(bind(socket_server,(*server_info).ai_addr, (*server_info).ai_addrlen) < 0){
    if (bind(socket_server, (struct sockaddr *) &addr_client ,sizeof(addr_client)) < 0)
    {
    	printf("Failed in socket binding");
    	system_error("BINDING");
    	exit(0);
    }

    printf("%s\n","Success: Socket Binded\n" );

    clients = (struct simple_broadcast_chat_server_client_user_information *)malloc(maximum_number_of_clients*sizeof(struct simple_broadcast_chat_server_client_user_information));
    clients_address = (struct sockaddr_in *)malloc(maximum_number_of_clients*sizeof(struct sockaddr_in));

    //listening phase
    if(listen (socket_server,10) < 0) {
    	printf("Fail: To client found\n");
    	system_error("LISTEN");
    	exit(0);
    }
    printf("%s\n","Listening to the client!" );

    //select - waiting for events
    FD_SET(socket_server, &fd_master); //add server socket to master set
    //keeping count of the file descriptors
    int max_fd = socket_server; //number of file descriptors should be highest file +1 
    printf("Max FD= %d\n",max_fd);
    int temp;

    while(1){
    	temp_fd = fd_master;
    	if (select(max_fd + 1,&temp_fd, NULL, NULL,NULL) == -1){
    		printf("%s\n","Error occured when selecting \n" );
    		system_error("SELECT:");
    		exit(0);
    	}
    	int i;
    	for ( i=0; i<=max_fd; i++){ //looping through the file descriptors
            // printf("Max FD = %d\n",max_fd);
    		if(FD_ISSET(i,&temp_fd)){ //taking one from existing file descriptors
    			if(i == socket_server){ //if this is server soket , then check for clients that wants to connect or send any message
    				//Accept and the address of the new client in the array
    				socklen_t size_client_address = sizeof(clients_address[number_of_clients]);
    				client_new = accept(socket_server, (struct sockaddr *)&clients_address[number_of_clients], &size_client_address );
                    // client_new = accept(socket_server,(struct sockaddr*)NULL, NULL);
    				if(client_new == -1){
    					printf("ERROR : Occured when accepting new client \n Error Number%d\n",(int)errno );
    				}else{
    					temp = max_fd;
    					FD_SET(client_new, &fd_master); //Adding the new client/connection to the connected clients' list

    					//updating the maximum number of file descriptors
    					if(client_new > max_fd){
    						max_fd = client_new;
    					}
    					if(number_of_clients + 1 > maximum_number_of_clients){
    						printf("ERROR: Exceeded the number of connected users\n Connection Denied \n");
    						max_fd = temp;
    						FD_CLR(client_new, & fd_master);
    						NACK_server_client(client_new);
    					}else{
    						if(client_join_check(client_new) == 0){
    							//new online client
    							//broadcast this information to other user
    							printf("USER : %s has connected and joined CHAT ROOM\n", clients[number_of_clients-1].username);
    							join_broadcast_message.header.version = 3;
    							join_broadcast_message.header.type = 8;
    							join_broadcast_message.attribute[0].type = 2;
    							strcpy(join_broadcast_message.attribute[0].payload_data, clients[number_of_clients-1].username)	;
    							//go through the file descriptor list and except the new client and server, broadcast the
    							//information of new client to everybody
    							int j;
    							for(j=0; j<=max_fd;j++){ //loop again the file descriptors and broadcast
    								if(FD_ISSET(j, &fd_master))
    								{
    									if(j != socket_server && j != client_new){
    										if((write(j,(void *)&join_broadcast_message,sizeof(join_broadcast_message))) == -1){
    											system_error("Error while broadcasting JOIN message");
    										}
    									}
    								}

    							}
    						} else{
    							max_fd = temp;
    							FD_CLR(client_new, &fd_master); //clear newclient if username is already used and hence not available 
    						}
    					}
    				}
    			}else{
    					//data from existing connection
    					number_of_bytes = read(i, (struct simple_broadcast_chat_server_message *)&receive_message,sizeof(receive_message));
                        // printf("Number of Bytes = %d\n",number_of_bytes );
    					if(number_of_bytes <= 0){
    						if(number_of_bytes == 0){
    							int k;
    							for(k=0; k < number_of_clients; k++){
    								if(clients[k].file_descriptor == i){
    									leave_broadcast_message.attribute[0].type = 2;
    									strcpy(leave_broadcast_message.attribute[0].payload_data,clients[k].username);
    								}
    							}
    							printf("User %s has left the CHATROOM\n", leave_broadcast_message.attribute[0].payload_data );
    							leave_broadcast_message.header.version = 3;
    							leave_broadcast_message.header.type = 6;
    							int j;
    							for (j = 0; j <=max_fd; j++){
    								if(FD_ISSET(j,&fd_master)){
    									if(j!=socket_server && j!=client_new){
    										if((write(j,(void*)&leave_broadcast_message,sizeof(leave_broadcast_message))) == -1){
    											system_error("BROADCASTING LEAVE MESSAGE");
    										}
    									}
    								}
    							}
    		                      printf("Fourth checkpoint\n");
    					}else if(number_of_bytes < 0){
    						// system_error("RECEIVING MESSAGE, WAITING");
                            printf("RECEIVING MESSAGE, WAITING\n");
    					}
    					close(i);
    					FD_CLR(i, &fd_master); //client is removed from the master set of connected clients
    					int x;
    					for(x=i; x<number_of_clients; x++){
    						clients[x] = clients[x+1];

    					}
    					number_of_clients--;
    				}else{
    					//number_of_bytes > 0
    					client_attribute = receive_message.attribute[0]; //gets message
    					forward_message = receive_message;
    					forward_message.header.type = 3;
    					forward_message.attribute[1].type =2;
    					//////check this line below : I thing needs to be changed to attribute[1]
    					forward_message.attribute[0].length = receive_message.attribute[0].length;
    					char name[16];
    					strcpy(name,receive_message.attribute[1].payload_data);

    					int k;
    					for(k=0; k<number_of_clients;k++){
    						if(clients[k].file_descriptor == i){
    							strcpy(forward_message.attribute[1].payload_data,clients[k].username);
    						}
    					}
    					printf("%s says %s\n",forward_message.attribute[1].payload_data, forward_message.attribute[0].payload_data );

    					//Forward the message to all the clients except the current one and server
    					int j;
    					for (j=0; j<=max_fd; j++){
    						//send forward message
    						if(FD_ISSET(j , &fd_master)){
    							if(j!=socket_server && j!=i){
    								//CHECK: I guess the condition should be false
    								if((write(j, (void*)&forward_message,number_of_bytes))==-1){
    									system_error("Forwarding message");
    								}
    							}
    						}
    					}
    				}//End forward message
    		}//End dealing with data from client
    	}else{
            printf("Garbage\n");}//end new connection
    }//end loop through file descriptors
} //while loop end


    close(socket_server);
    return 0;
}