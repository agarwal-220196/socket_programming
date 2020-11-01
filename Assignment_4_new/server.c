/*********************************HTTP server implementation*************/
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

int main(int argc, char *argv[])
{
	char ack_request[1024];
	time_t now;
	struct tm *timenow;
	struct addrinfo hint, *res;
	int send_length;
	FILE *fp;
	int receive_length = -1;
	int listener_filedescriptor, client_filedescriptor;
	struct sockaddr_storage remote_addr;
	struct sockaddr_in addr;
	socklen_t sin_size;
	fd_set set_1;
	fd_set set_2;
	int maximum_filedescriptor;
	int socket_ITR;
	char ac_Buffer[BUFFER_SIZE], ac_Host[NAME_LENGTH],ac_u_r_l[NAME_LENGTH], ac_Name[NAME_LENGTH];
	int index_in_cache = -1;
	int port_http = PORT_NO;
	int proxy_filedescriptor;
	int i, old_destination_entry =0;
	FILE *read_filepointer;
	char *expire = NULL;
	char *pc_token = NULL;
	int read_length =0;
	char modified [100];
	char modified_request[BUFFER_SIZE];

	memset(cache,0,10*sizeof(_cache));
	if(argc < 3)
	{
		printf("\n WRONG USAGE: ./<server> <host> <port> \n");
		exit(-1);
	}
	memset(&remote_addr,0, sizeof remote_addr);
	memset(&addr,0,sizeof addr);

	port_http = atoi(argv[2]);
	addr.sin_port = htons(port_http);
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_family = AF_INET;

	if((listener_filedescriptor = socket(AF_INET, SOCK_STREAM, 0)) <0)
	{
		system_error("ERROR: In creating socket");
		//exit(-1);
	}
	if(bind(listener_filedescriptor,(struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0)
	{
		system_error("ERROR: In bind socket");
		exit(-1);
	}
	if(listen(listener_filedescriptor,10) <0)
	{
		system_error("ERROR: In listen");
		exit(-1);
	}

	FD_SET(listener_filedescriptor, &set_1);
	maximum_filedescriptor= listener_filedescriptor;

	memset(cache,0,10*sizeof(_cache));

	printf("WAITING... for request1 \n");
	sin_size = sizeof(remote_addr);

	while(1)
	{
		set_2= set_1;
		if(select(maximum_filedescriptor+1, &set_2, NULL, NULL, NULL) == -1){
			system_error("ERROR: In select");
			exit(-1);
		}
		for (socket_ITR = 0; socket_ITR <=maximum_filedescriptor; socket_ITR++)
		{
			if(FD_ISSET(socket_ITR, &set_2))
			{
				if(socket_ITR == listener_filedescriptor)
				{
					client_filedescriptor= accept(listener_filedescriptor,(struct sockaddr*)&remote_addr,&sin_size);
					maximum_filedescriptor= client_filedescriptor>maximum_filedescriptor ? client_filedescriptor : maximum_filedescriptor;
					FD_SET(client_filedescriptor, &set_1);
					if(client_filedescriptor == -1)
					{
						system_error("ERROR: In accept");
						continue;
					}
				}
				else
				{
					memset(ac_Buffer, 0, BUFFER_SIZE);
					memset(ac_Host, 0, NAME_LENGTH);
					memset(ac_u_r_l, 0, NAME_LENGTH);
					memset(ac_Name, 0, NAME_LENGTH);

					client_filedescriptor= socket_ITR;
					if(recv(client_filedescriptor, ac_Buffer, sizeof(ac_Buffer),0) < 0)
					{
						system_error("ERROR: In recv");
						close(client_filedescriptor);
						return 1;
					}
					printf("Message sent from CLIENT : \n%s \n", ac_Buffer);
					if(parse_read_request(ac_Buffer, ac_Host, &port_http, ac_u_r_l, ac_Name)!=4)
					{
						send_error_message(400, client_filedescriptor);
						close(client_filedescriptor);
						return 1;
					}
					if((proxy_filedescriptor = socket(AF_INET, SOCK_STREAM, 0)) <0){
						system_error("ERROR: In PROXYSOCKET ERROR");
						close(client_filedescriptor);
						return 1;
					}

					memset(&hint, 0, sizeof(hint));
					hint.ai_family = AF_INET;
					hint.ai_socktype = SOCK_STREAM;

					if(getaddrinfo(ac_Host,"80", &hint, &res)!=0)
					{
						printf("ERROR: GETADDRINFO\n");
						send_error_message(404, client_filedescriptor);
						close(client_filedescriptor);
						return 1;
					}
					if(connect(proxy_filedescriptor, res->ai_addr, res->ai_addrlen) < 0)
					{
						close(proxy_filedescriptor);
						system_error("ERROR: IN connect");
						send_error_message(404, client_filedescriptor);
						close(client_filedescriptor);
						return 1;
					}
					time(&now);
					timenow = gmtime(&now);

					index_in_cache = check_if_cache_entry_present(ac_u_r_l);
					if(index_in_cache == -1)
					{
						//new entry in cache. 
						//Request is sent to HTTP server
						memset(ack_request, 0, 1024);
						printf("NOT FOUND IN CACHE...\n Downloading from %s:%d\n",ac_Host, port_http );
						printf("\n SENDING ... HTTP QUERY -> WEB SERVER");
						sprintf(ack_request, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", ac_Name, ac_Host);
						puts(ack_request);
						if((send_length = send(proxy_filedescriptor, ack_request, strlen(ack_request),0)) <0)
						{
							system_error("ERROR: in send request");
							close(proxy_filedescriptor);
							return 1;
						}
						printf("REQUEST SENT -> SERVER \n");
						memset(ac_Buffer ,0, BUFFER_SIZE);
						for (i = 0; i< 10; i++)
						{
							if(cache[i].Is_filled == 0)
							{
								old_destination_entry = i;
								break;
							}
							else
							{
								if(time_comparison(cache[i].Last_Modfd, cache[old_destination_entry].Last_Modfd) <= 0)
								{
									old_destination_entry = i;
								}

							}
						}
						memset(&cache[old_destination_entry], 0, sizeof(_cache));
						cache[old_destination_entry].Is_filled = 1;

						pc_token= strtok(ac_Name,"/");

						while(pc_token != NULL)
						{
							strcpy(cache[old_destination_entry].Fname, pc_token);
							pc_token = strtok(NULL, "/");
						}

						memcpy(cache[old_destination_entry].u_r_l, ac_u_r_l, NAME_LENGTH);
						sprintf(cache[old_destination_entry].Last_Modfd,"%s, %02d %s %d %02d:%02d:%02d GMT", day[timenow->tm_wday], timenow ->tm_mday, month[timenow ->tm_mon], timenow->tm_year+ 1900, timenow->tm_hour, timenow->tm_min, timenow->tm_sec  );

						remove(cache[old_destination_entry].Fname);
						fp = fopen(cache[old_destination_entry].Fname, "w");
						if(fp == NULL)
						{
							printf("ERROR : Failed to create CACHE\n");
							return 1;
						}
						while((receive_length = recv(proxy_filedescriptor, ac_Buffer, BUFFER_SIZE, 0)) > 0)
						{
							if(send(client_filedescriptor, ac_Buffer, receive_length,0) <0)
							{
								system_error("ERROR: Client send");
								return 1;
							}
							fwrite(ac_Buffer, 1, receive_length, fp);
							memset(ac_Buffer, 0, BUFFER_SIZE);
						}
						send(client_filedescriptor, ac_Buffer, 0, 0);
						printf("SUCCESS: Received response from server\n");
						printf("SENT FILE TO THE CLIENT \n ");

						fclose(fp);

						read_filepointer = fopen(cache[old_destination_entry].Fname, "r");
						printf("old : %d\n",old_destination_entry);
						fread(ac_Buffer, 1, 2048, read_filepointer);
						//fwrite(ac_Buffer, 1, 2048, read_filepointer);
						fclose(read_filepointer);

						expire = strstr(ac_Buffer, "EXPIRES: ");
						if(expire != NULL)
						{
							memcpy(cache[old_destination_entry].Exp, expire+9, 29);
						}
						else
						{
							sprintf(cache[old_destination_entry].Exp, "%s, %02d %s %d %02d:%02d:%02d GMT", day[timenow->tm_wday], timenow ->tm_mday, month[timenow ->tm_mon], timenow->tm_year+ 1900, timenow->tm_hour, (timenow->tm_min)+2, timenow->tm_sec );			
						}
					}
					else
					{
						//printf("First CHECKPOINT \n\n");
						if(check_if_cache_entry_expire(ac_u_r_l, timenow) >=0)
						{
							printf("FILE FOUND: in cache AND NOT EXPIRED\n Sending file to client..\n");
							sprintf(cache[index_in_cache].Last_Modfd,"%s, %02d %s %d %02d:%02d:%02d GMT", day[timenow->tm_wday], timenow ->tm_mday, month[timenow ->tm_mon], timenow->tm_year+ 1900, timenow->tm_hour, timenow->tm_min, timenow->tm_sec  );
							read_filepointer = fopen(cache[index_in_cache].Fname, "r");
							printf("Index in cache:  %d\n", index_in_cache);
							memset(ac_Buffer, 0, BUFFER_SIZE);
							//printf("Outside while\n");
							while((read_length= fread(ac_Buffer,1 , BUFFER_SIZE, read_filepointer)) > 0)
							{	//printf("Inside while before send\n");
								send(client_filedescriptor, ac_Buffer, read_length, 0);
							}
							printf("SUCCESS: SENT FILE TO CLIENT");
							fclose(read_filepointer);
						}
						else
						{
							printf("FILE EXPIRED. REQUESTING UPDATED FILE (from SERVER) \n");
							memset(modified, 0, 100);
							sprintf(modified, "If-Modified-Since: %s\r\n\r\n", cache[index_in_cache].Last_Modfd);
							memset(modified_request, 0, BUFFER_SIZE);
							memcpy(modified_request, ac_Buffer, strlen(ac_Buffer)-2);

							strcat(modified_request, modified);
							printf("SENDING HTTP QUERY -> WEB SERVER\n");
							printf("%s\n", modified_request);
							send(proxy_filedescriptor, modified_request, strlen(modified_request),0);
							memset(ac_Buffer,0, BUFFER_SIZE);
							receive_length = recv(proxy_filedescriptor, ac_Buffer, BUFFER_SIZE, 0);
							expire =  strstr(ac_Buffer, "Expires : ");
							if(expire != NULL)
							{
								memcpy(cache[index_in_cache].Exp, expire+9, 29);
							}
							else
							{
								sprintf(cache[index_in_cache].Exp, "%s, %02d %s %d %02d:%02d:%02d GMT", day[timenow->tm_wday], timenow ->tm_mday, month[timenow ->tm_mon], timenow->tm_year+ 1900, timenow->tm_hour, timenow->tm_min, timenow->tm_sec );

							}
							if(receive_length > 0)
							{
								printf("HTTP RESPONSE: \n %s\n",ac_Buffer );
								if((*(ac_Buffer + 9) == '3') && (*(ac_Buffer + 10) == '0') && (*(ac_Buffer + 11) == '4')) //3040response <---BONUS IMPLEMENTED
								{
									printf("FILE UPTO DATE. sending file from cache\n");
									read_filepointer = fopen(cache[index_in_cache].Fname, "r");
									memset(ac_Buffer, 0, BUFFER_SIZE);
									while((read_length = fread(ac_Buffer, 1, BUFFER_SIZE, read_filepointer)) >0)
									{
										send(client_filedescriptor, ac_Buffer, read_length, 0);
									}
									fclose(read_filepointer);
								}	
								else if ((*(ac_Buffer+9) == '4') && (*(ac_Buffer +10 ) =='0') &&(*(ac_Buffer+11) == '4'))
								{
									send_error_message(404, client_filedescriptor);
								}
								else if((*(ac_Buffer+9) =='2') && (*(ac_Buffer +10) == '0') && (*(ac_Buffer +11) == '0'))
								{
									printf("NEW FILE RECEIVED FROM SERVER.\n UPDATE CACHE AND SEND FILE TO CLIENT\n");
									send(client_filedescriptor, ac_Buffer, receive_length, 0);
									remove(cache[index_in_cache].Fname);

									expire = NULL;
									expire = strstr(ac_Buffer, "EXPIRES:");
									if(expire != NULL)
									{
										memcpy(cache[index_in_cache].Exp, expire+9,29);
									}
									else
									{
										sprintf(cache[index_in_cache].Exp, "%s, %02d %s %d %02d:%02d:%02d GMT", day[timenow->tm_wday], timenow ->tm_mday, month[timenow ->tm_mon], timenow->tm_year+ 1900, timenow->tm_hour, timenow->tm_min, timenow->tm_sec );

									}
									sprintf(cache[index_in_cache].Last_Modfd, "%s, %02d %s %d %02d:%02d:%02d GMT", day[timenow->tm_wday], timenow ->tm_mday, month[timenow ->tm_mon], timenow->tm_year+ 1900, timenow->tm_hour, timenow->tm_min, timenow->tm_sec );
									fp = fopen(cache[index_in_cache].Fname, "w");
									fwrite(ac_Buffer, 1, receive_length, fp);

									memset(ac_Buffer, 0, BUFFER_SIZE);
									while((receive_length = recv(proxy_filedescriptor, ac_Buffer, BUFFER_SIZE, 0)) > 0)
									{
										send(client_filedescriptor, ac_Buffer, receive_length,0);
										fwrite(ac_Buffer, 1, receive_length, fp);
									}
									fclose(fp);

								}
							}
							else
							{
								system_error("ERROR: In receive");
							}
						}
					}
					FD_CLR(client_filedescriptor, &set_1);
					close(client_filedescriptor);
					close(proxy_filedescriptor);
					printf("\n printing cache table \n");
					for (i=0;i<10;i++)
					{
						if(cache[i].Is_filled)
						{
							printf("Cache Entry Number %d\n", i + 1 );
							printf("URL              : %s\n",cache[i].u_r_l);
							printf("Last Access Time : %s\n",cache[i].Last_Modfd);
							printf("Expiry           : %s\n", cache[i].Exp );
							printf("File Name        : %s\n", cache[i].Fname);
							printf("====================================================\n");
						}
					}
				}
			}
		}

	}
}