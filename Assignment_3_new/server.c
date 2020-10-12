/*********************************TFTP server implementation*************/
//Author: Sanket Agarwal and Dhiraj Kudva
//Organisation: Texas A&M university
//Usage: Used to handle the TFTP requests from the client end. 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define MAX_SIZE 512

int system_error(const char *x)
{
	perror(x);
	exit(1);
}

int timeout(int file_descriptor, int time){
	
	fd_set receive_set;
	struct timeval time_value;
	FD_ZERO(&receive_set);
	FD_SET (file_descriptor, &receive_set);
	time_value.tv_sec = time;
	time_value.tv_usec = 0;
	
	return (select (file_descriptor + 1, &receive_set, NULL, NULL, &time_value)); //timeout will be called with 1 sec as described in the pdf. 

	
}

int main (int argc, char *argv[]){
	
	char data_read[512] = {0}; //initialising it to zero.
	int socket_file_descriptor, new_socket_file_descriptor;
	struct sockaddr_in client;
	socklen_t length_client = sizeof(struct sockaddr_in);
	
	int g, return_get_addr;
	int send_response;
	int last;
	
	int return_value, receive_byte;
	char buffer[1024] = {0};
	char ack[32] = {0};
	char payload[516] = {0};
	char payload_copy[516] = {0};
	char file_name[MAX_SIZE];
	char mode[512];
	
	unsigned short int op_code1, op_code2, block_number;
	int b,j;//general variables used inside for loops. 
	
	FILE *file_pointer;
	
	struct addrinfo address_dynamic, *addi, *client_info, *p;
	int true_yes;
	int pid;
	int read_return;
	int block_num=0;
	char ip[INET6_ADDRSTRLEN];
	int timeout_c = 0;
	int count =0;
	char c;
	char next_character = -1;
	int neg_ack = 0;
	char *ephemeral_pt;
	
	ephemeral_pt = malloc (sizeof ephemeral_pt);
	
	if (argc!=3)
	{
		system_error ("Incorrect USage: ./server <ip> <portnumber>");
		return 0;
	}
	
	true_yes = 1;
  //socklen_t addrlen;
  //configuring the address of the socket. 
	memset(&address_dynamic, 0, sizeof address_dynamic);
  address_dynamic.ai_family   = AF_INET;
  address_dynamic.ai_socktype = SOCK_DGRAM;
  address_dynamic.ai_flags    = AI_PASSIVE;
  
  // get the address information and report error in case of issues with the acutal issue. 
   if ((return_get_addr = getaddrinfo(NULL, argv[2], &address_dynamic, &addi)) != 0) {
    fprintf(stderr, "error server: %s\n", gai_strerror(return_get_addr));
    exit(1);
  }
  
    for(p = addi; p != NULL; p = p->ai_next) {
    socket_file_descriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (socket_file_descriptor < 0) {
      continue;//do not go to setup the socket. 
    }
    setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &true_yes, sizeof(int));
    if (bind(socket_file_descriptor, p->ai_addr, p->ai_addrlen) < 0) {
      close(socket_file_descriptor); // close because of bind failure. 
      continue;
    }
    break;// exit with the socket being binded. 
  }
  
  freeaddrinfo(addi);// release the address.
  printf("Server is now read. Waiting for Clients to join...\n");
  
  while(1){
	  //get data from the respective socket. and store it in a buffer. 
	  receive_byte = recvfrom(socket_file_descriptor, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &length_client);
	  
	  //check if the number of recived bytes are zero or not. 
	if(receive_byte<0){
		system_error("Error: receive bytes unsucessful");
		return 0;
	  }
	  //else bytes are received correctly proceed ahead. 
	  
	memcpy (&op_code1, &buffer, 2);
	op_code1 = ntohs(op_code1);
	
	pid = fork();//create the child. 
	
	if(pid==0){//child process...
		
		
		if (op_code1 ==1 ){// RRQ process, do according.
			//initialize to zero first. 
			
			bzero(file_name, MAX_SIZE);
			
			// check for the EOF character using a for loop.
			
			for (b=0; buffer[2+b]!='\0'; b++)
			{
				file_name[b] = buffer[2+b];
			}
			//once you get the file name add a eof to the file name. b is already incremented to the position before exiting so can be added at the same location
			
			file_name[b] = '\0';
			bzero(mode, 512);
			g=0; // will be used to keep track of the mode counter. 
			
			for (j = b+3; buffer[j] != '\0'; j++) {    // reading until '\0' of mode. 
				mode[g]=buffer[j];
				g++;
			}	
			mode[g]='\0';// same logic as the file_name mentioned above on line 147
			printf("RRQ: filename: %s mode: %s\n", file_name, mode);
			
			// open the file now. 
			
			file_pointer = fopen(file_name, "r");
			
			if (file_pointer!=NULL){//i.e. file is opened now. 
				
				close(socket_file_descriptor);// because we dont want to resend on the same port. we are sending back on an ephemeral_pt
				*ephemeral_pt = htons(0);
				
				if ((return_get_addr = getaddrinfo(NULL, ephemeral_pt, &address_dynamic, &client_info)) != 0) {
					fprintf(stderr, "return_get_addr: %s\n", gai_strerror(return_get_addr));
					return 1;
				}
				
				for(p = client_info; p != NULL; p = p->ai_next) {
					if ((new_socket_file_descriptor = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {// same as the above one. 
					system_error("Error: SERVER (child): new socket not created");
					continue;
					}
            
					setsockopt(new_socket_file_descriptor,SOL_SOCKET,SO_REUSEADDR,&true_yes,sizeof(int));
					if (bind(new_socket_file_descriptor, p->ai_addr, p->ai_addrlen) == -1) {
						close(new_socket_file_descriptor);
						system_error("Error: SERVER (new_socket_file_descriptor): bind issues.");
						continue;
					}
					break;
				}	
				freeaddrinfo(client_info);// same as above. 
				//debug
				//printf("Checkpoint1010: reached new socket creation.");
				
				// now we need to create the datapacket to send back on the newly created socket.
				
				bzero(payload, sizeof(payload));//initialize to zero initially. 
				bzero(data_read, sizeof(data_read));
				
				//read from the file now. 
				read_return = fread(&data_read,1,512,file_pointer);
				
				if (read_return>=0){
					data_read[read_return] = '\0'; //same logic as above. 
					printf("read result is %d\n",read_return);
				}
				
				if(read_return < 512)
					last = 1;
				
				block_number = htons(1);                            //1st block 
				op_code2 = htons(3);                                // 3 is data 
				memcpy(&payload[0], &op_code2, 2);             // 2 Bytes
				memcpy(&payload[2], &block_number, 2);         // 4th Byte 
			
				for (b = 0; data_read[b] != '\0'; b++) {        
					payload[b+4] = data_read[b];// concating the data. 
				}
				
				bzero(payload_copy, sizeof(payload_copy)); // reset it to zero. 
				memcpy(&payload_copy[0], &payload[0], 516); // copy all the bytes. 
				send_response = sendto(new_socket_file_descriptor, payload, (read_return + 4), 0, (struct sockaddr*)&client, length_client);
				
				neg_ack = 1;
				if (send_response < 0)
					system_error("ERROR to send first packet: \n");
				else 
					printf("First block successfully sent .\n");

				while(1){
					if(timeout(new_socket_file_descriptor,1)!= 0){
						bzero(buffer,sizeof(buffer));
						bzero(payload,sizeof(payload));
						receive_byte = recvfrom(new_socket_file_descriptor, buffer, sizeof(buffer), 0, (struct sockaddr*)&client,&length_client);
						timeout_c = 0;
						if(receive_byte < 0){
							system_error("ERROR: Data not received");
							return 6;
						}
						else{
							printf("RECEIVED DATA");
						}
						memcpy(&op_code1, &buffer[0],2);
						if(ntohs(op_code1)==4){					//OPCODE = 4; ACK
							bzero(&block_num, sizeof(block_num));
							memcpy(&block_num,&buffer[2],2);
							block_num = ntohs(block_num);
							printf("Acknowledgement %i received\n",block_num);
							//EOF check
							if(block_num == neg_ack){
								printf("Expected Acknowledgement has arrived\n");
								neg_ack = (neg_ack + 1)%65536;
								if(last == 1){
									close(new_socket_file_descriptor);
									fclose(file_pointer);
									printf("SERVER: Full file sent completely \n");
									exit(5);
									last = 0;
								}
								else{
									bzero(data_read,sizeof(data_read));
									read_return = fread (&data_read,1,512,file_pointer);
									if(read_return >= 0){
										if(read_return < 512)
											last = 1;
										data_read[read_return]='\0';
										block_number = htons(((block_num+1)%65536)); 
										op_code2 = htons(3);
										memcpy(&payload[0],&op_code2,2);
										memcpy(&payload[2],&block_number,2);

										//for(b=0;data_read[b] != '\0';b++){
										//	payload[b+4] = data_read[b];
										//}

										//bzero(payload_copy,sizeof(payload_copy));
										printf("actual bytes read from file %d",read_return);
										memcpy(&payload[4],data_read, read_return);
										int send_response = sendto(new_socket_file_descriptor,payload,(read_return+4),0,(struct sockaddr*)&client, length_client);

										if(send_response < 0)
											system_error("ERROR: send to");
									}
								}
							}
							else {
								printf("ERROR: Expected ACK has not arrived :NEG ACK = %d, Block number =%d\n", neg_ack,block_number);
							}
						}
					}
					else{
						timeout_c ++ ;
						printf("TIMEOUT OCCURED\n");
						if(timeout_c == 10){
							printf("TIMEOUT occured 10 times. Closing socket connection with client\n");
							close(new_socket_file_descriptor);
							fclose(file_pointer);
							exit(6);
						}
						else{
							bzero(payload,sizeof(payload));
							memcpy(&payload[0],&payload_copy[0],516);
							memcpy(&block_number,&payload[2],2);
							block_number = htons(block_number);
							printf("RETRANSMIT: Data with Block number : %d\n",block_number );
							send_response = sendto(new_socket_file_descriptor,payload_copy,(read_return+4),0,(struct sockaddr*)&client,length_client);
							bzero(payload_copy,sizeof(payload_copy));
							memcpy(&payload_copy[0],&payload[0],516);

							if(send_response < 0)
								system_error("ERROR: In sendto ");
							}		
						}
					}
				}
				else {
					unsigned short int ERROR_CODE = htons(1);
					unsigned short int ERROR_OPCODE = htons(5);
					char ERROR_MESSAGE[512] = "File not found";
					char ERROR_BUFFER [516] = {0};
					memcpy(&ERROR_BUFFER[0],&ERROR_OPCODE,2);
					memcpy(&ERROR_BUFFER[2],&ERROR_CODE,2);
					memcpy(&ERROR_BUFFER[4], &ERROR_MESSAGE, 512);
					sendto(socket_file_descriptor,ERROR_BUFFER, 516, 0 ,(struct sockaddr*)&client, length_client);
					printf("SERVER CLEANUP: File names do not match \n");
					close(socket_file_descriptor);
					fclose(file_pointer);
					exit(4);
				}
			}
			else if(op_code1 == 2){
				*ephemeral_pt = htons(0);
				if((return_get_addr = getaddrinfo(NULL, ephemeral_pt, &address_dynamic, &client_info)) != 0){
					fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(return_get_addr) );
					return 10;
				}
				for (p = client_info; p != NULL; p = p->ai_next){
					if((new_socket_file_descriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol))== -1){
						system_error("ERROR: SERVER: child socket");
						continue;
					}
					setsockopt(new_socket_file_descriptor,SOL_SOCKET,SO_REUSEADDR,&true_yes, sizeof(int));
					if(bind(new_socket_file_descriptor, p->ai_addr, p->ai_addrlen) == -1){
						close(new_socket_file_descriptor);
						system_error("ERROR: SERVER: bind new socket");
						continue;
					}
					break;
				}
				freeaddrinfo(client_info);
				printf("WRQ: Request received to SERVER from client\n");
							bzero(file_name, MAX_SIZE);
			
			// check for the EOF character using a for loop.
			
			for (b=0; buffer[2+b]!='\0'; b++)
			{
				file_name[b] = buffer[2+b];
			}
			//once you get the file name add a eof to the file name. b is already incremented to the position before exiting so can be added at the same location
			
			file_name[b] = '\0';
				printf("FILENAME : %s\n",file_name );
				FILE *file_pointer_write = fopen(file_name, "w+"); //check this here
				if(file_pointer_write == NULL)
				{
					printf("SERVER:WRQ: File cannot be opened\n");
				}
				op_code2 = htons(4);
				block_number =  htons(0);
				bzero(ack, sizeof(ack));
				memcpy(&ack[0],&op_code2,2);
				memcpy(&ack[2],&block_number, 2);
				send_response = sendto(new_socket_file_descriptor, ack, 4, 0, (struct sockaddr*)&client, length_client);
				neg_ack = 1;
				if(send_response < 0)
					system_error("ERROR: WRQ: ACK: sendto");
				while(1){
					bzero(buffer, sizeof(buffer));
					receive_byte = recvfrom(new_socket_file_descriptor, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &length_client);
					if(receive_byte < 0){
						system_error("ERROR:WRQ: Data not received");
						return 9;
					}
					bzero(data_read, sizeof(data_read));
					memcpy(&block_number,&buffer[2],2);
					g=0;
					for(b =0; buffer[b+4]!='\0';b++){
						if(buffer[b+4] == '\n'){
							printf("LF CHARACTER PRESENT\n");;
							g++;
							if(b-g < 0)
								printf("ERROR: b - g less than zero\n");
							data_read[b-g] = '\n';
						}
						else{
							data_read[b-g] = buffer[b+4];
						}
					}
					fwrite(data_read,1, (receive_byte- 4 - g),file_pointer_write);
					block_number = ntohs(block_number);

					if(neg_ack == block_number){
						printf("SERVER: DATA RECEiVED: BLOCK NUMBER :%d\n",neg_ack );
						printf("SERVER: DATA EXPECTED received\n");
						op_code2 = htons(4);
						block_number = ntohs(neg_ack);
						bzero(ack,sizeof(ack));
						memcpy(&ack[0],&op_code2,2);
						memcpy(&ack[2],&block_number,2);
						printf("SERVER ACK SENT for block number %d\n",htons(block_number) );
						send_response = sendto(new_socket_file_descriptor,ack,4,0,(struct sockaddr*)&client, length_client);
						if(receive_byte < 516){
							printf("Last Data block has arrived.Closing connection\n");
							close(new_socket_file_descriptor);
							fclose(file_pointer_write);
							exit(9);

						}
						neg_ack = (neg_ack + 1)%65536;
					}
				}
			}
		
		}
	}
	
  }
  

