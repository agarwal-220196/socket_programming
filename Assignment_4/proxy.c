/*********Proxy code****************/
/*Authors: Sanket Agarwal and Dhiraj Kudva
Organization: Texas A&M University
Description: Client code to access the proxy server. Takes in the url from the client.*/

//including the library files. these are standard library files. 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
#include <time.h>
#include <common_def.h>

struct http_cache {
	char URL_NAME [256];
	char LAST_MOD [50];
	char LAST_ACCESS_DATE [50];
	char EXPIRE [50];
	char *body;
};

static const struct http_cache clear;
int number_of_cache_entries = 0;

struct http_cache cache_proxy [CACHE_MAX];

int cache_update(char *URL_NAME, char* buffer, int flag_signal, int x){
	int j=0;
	int p=0;

	if(flag_signal == 1){  //NEW
		if(number_of_cache_entries == CACHE_MAX){
			cache_proxy[0] = clear; //LRU popping
			for (j = 0; j < CACHE_MAX ; j++){
				if (j+1 != CACHE_MAX)
					cache_proxy [j] = cache_proxy[j+1];
				else{
					//HEAD :  Adding new entry
					memset(&cache_proxy[j], 0 , sizeof(struct http_cache));
					memcpy(cache_proxy[j].URL_NAME, URL_NAME, 256);
					cache_proxy[j].body = (char *)malloc(strlen(buffer));
					memcpy(cache_proxy[j].body, buffer, strlen(buffer));
					parser_proxy("EXPIRE : ",buffer,cache_proxy[j].EXPIRE);
					parser_proxy("LAST_MOD :",buffer,cache_proxy[j].LAST_MOD);
					parser_proxy("DATE : ",buffer,cache_proxy[j].LAST_ACCESS_DATE);
				}
			}
		}
		else { 
			//WHEN CACHE HAS NOT YET REACHED MAX ENTRY CAPACITY
			cache_proxy[number_of_cache_entries] = clear;
			memcpy(cache_proxy[number_of_cache_entries].URL_NAME, URL_NAME, 256);
			parser_proxy("EXPIRE : ",buffer,cache_proxy[number_of_cache_entries].EXPIRE);
			parser_proxy("LAST_MOD :",buffer,cache_proxy[number_of_cache_entries].LAST_MOD);
			parser_proxy("DATE : ",buffer,cache_proxy[number_of_cache_entries].LAST_ACCESS_DATE);
			cache_proxy[number_of_cache_entries].body = (char*)malloc(strlen(buffer));
			memcpy(cache_proxy[number_of_cache_entries].body, buffer, strlen(buffer));
			number_of_cache_entries ++ ;

		}
	}
	else {	//EXISTING
		struct http_cache temp;
		memset(&temp, 0 , sizeof(struct http_cache));
		temp = cache_proxy[x];
		for (j=x; j<number_of_cache_entries;j++){
			if(j==number_of_cache_entries - 1)
				break;
			cache_proxy[j] = cache_proxy[j+1];
		}
		cache_proxy[number_of_cache_entries - 1]=temp;
		struct tm temp_time;
		time_t new_time = time(NULL);
		temp_time = *gmtime(&new_time);
		const char* operate_time = "%a, %d %b %Y %H:%M:%S GMT";
		strftime(cache_proxy[number_of_cache_entries -1].LAST_ACCESS_DATE, 50, operate_time, &temp_time);
	}
}
int display_cache(){
	int t = 0;
	if(number_of_cache_entries == 0)
		printf("INFO: NO CACHE ENTRY. CACHE UNOCCUPIED\n");
	else{
		printf("NUMBER OF ENTRIES IN CACHE : %d\n", number_of_cache_entries);
		for (t= 0;t < number_of_cache_entries;t++){
			if(strcmp(cache_proxy[t].EXPIRE,"")!=0 && strcmp(cache_proxy[t].LAST_MOD, "")!=0)
				printf("Index : %d || URL : %s || ACCESS DATE: %s || EXPIRE : %s || LAST MODIFIED : %s \n\n",t,cache_proxy[t].URL_NAME,cache_proxy[t].LAST_ACCESS_DATE, cache_proxy[t].EXPIRE, cache_proxy[t].LAST_MOD);
			else if(strcmp(cache_proxy[t].EXPIRE,"")==0 && strcmp(cache_proxy[t].LAST_MOD, "")==0)
				printf("Index : %d || URL : %s || ACCESS DATE: %s || EXPIRE : N/A || LAST MODIFIED : N/A \n\n",t,cache_proxy[t].URL_NAME,cache_proxy[t].LAST_ACCESS_DATE, cache_proxy[t].EXPIRE, cache_proxy[t].LAST_MOD);
			else if(strcmp(cache_proxy[t].EXPIRE, "") == 0)
				printf("Index : %d || URL : %s || ACCESS DATE: %s || EXPIRE : N/A || LAST MODIFIED : %s \n\n",t,cache_proxy[t].URL_NAME,cache_proxy[t].LAST_ACCESS_DATE, cache_proxy[t].EXPIRE, cache_proxy[t].LAST_MOD);
			else if(strcmp(cache_proxy[t].LAST_MOD,"")==0)
				printf("Index : %d || URL : %s || ACCESS DATE: %s || EXPIRE : %s || LAST MODIFIED : N/A \n\n",t,cache_proxy[t].URL_NAME,cache_proxy[t].LAST_ACCESS_DATE, cache_proxy[t].EXPIRE, cache_proxy[t].LAST_MOD);
		}
	}
	return 0;
}
int fresh_cache (int cache_pointer){
	struct tm temp_time;
	time_t new_time = time(NULL);
	temp_time = *gmtime(&new_time);
	struct tm EXPIRES;
	if(strcmp(cache_proxy[cache_pointer].EXPIRE ,"")!= 0){
		strptime(cache_proxy[cache_pointer].EXPIRE, "%a, %d %b %Y %H:%M:%S %Z", &EXPIRES);
		time_t expired = mktime(&EXPIRES);
		time_t current = mktime(&temp_time);
		if (difftime(current ,expired) <0)
			return 1;
		else
			return -1;
	}
	else
		return -1;
}	

int element_cache(char *URL_NAME){
	int b=0;
	for(b=0; b<CACHE_MAX; b++){
		if(strcmp(cache_proxy[b].URL_NAME, URL_NAME) == 0){
			return b;
		}
	}
	return -1;
}

int socket_web(char *host_name){
	struct addrinfo dynamic_addr, *ai, *p;
	int return_value = 0;
	int socket_web_file_descriptor = 0;

	memset(&dynamic_addr, 0, sizeof(dynamic_addr));
	dynamic_addr.ai_family = AF_INET;
	dynamic_addr.ai_socktype = SOCK_STREAM;
	if((return_value = getaddrinfo (host_name, "http", &dynamic_addr,&ai)) !=0 ){
		fprintf(stderr, "SERVER :%s\n", gai_strerror(return_value));
		exit(1);
	}
	for(p = ai ; p != NULL; p = p->ai_next){
		socket_web_file_descriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(socket_web_file_descriptor >=0 && (connect(socket_web_file_descriptor,p->ai_addr,p->ai_addrlen) >=0))
			break;
	}
	if(p == NULL)
		socket_web_file_descriptor = -1;
		freeaddrinfo(ai);
		return socket_web_file_descriptor;
}

int server_proxy(int client_file_descriptor){
	int socket_web_file_descriptor;
	char *message;
	char message_client_forward [LENGTH_MAX]={0};
	int return_value;
	int el_cache = 0;
	char *response = NULL;
	char *to_client = NULL;
	char http_path [256];
	char http_host_name[64];
	int http_port = 80;
	char URL_NAME[256]	 = {0};
	char method[8] = {0};
	char protocol [16]={0};
	char condition_message[256] = {0};
	char parsed_url[256] = {0};
	int checksum =0;

	message = (char*)malloc(LENGTH_MAX);
	return_value = read(client_file_descriptor, message, LENGTH_MAX);
	printf("SERVER : Request retrieved from CLIENT  : %s\n", message);
	if(return_value < 0)
		system_error("ERROR: At SERVER: in extracting message from client");
	sscanf(message, "%s %s %s" ,method, URL_NAME, protocol);

	if((el_cache = element_cache(URL_NAME))!= -1 && (fresh_cache(el_cache) == 1)){
		printf("INFO: SERVER: Requested URL : %s is present in cache and is FRESH \n",URL_NAME);
		cache_update(URL_NAME, NULL, 0, el_cache);
		to_client = (char*)malloc(strlen(cache_proxy[el_cache].body));
		memcpy(to_client,cache_proxy[el_cache].body, strlen(cache_proxy[el_cache].body));
	}
	else { 
		memset(http_host_name,0,64);
		memset(http_path,0,256);
		memcpy(&parsed_url[0],&URL_NAME[0],256);
		parser_client(parsed_url, http_host_name, &http_port, http_path);
		if((socket_web_file_descriptor = socket_web (http_host_name)) == -1)
			system_error("ERROR: In server connecting with web server");
		printf("SERVER: SUCCESS: Connected to the webserver %d\n", socket_web_file_descriptor);

		if(el_cache != -1){
			printf("SERVER : Requested URL : %s is in cache but EXPIRED\n",URL_NAME);
			if(strcmp(cache_proxy[el_cache].EXPIRE ,"")!=0 && strcmp(cache_proxy[el_cache].LAST_MOD , "")!=0)
				snprintf(condition_message, LENGTH_MAX, "%s %s %s \r \n HOST : %s \r \n USER-AGENT: HTTPTool/1.0 \r\n If-MODIFIED_SINCE: %s \r\n\r\n", method, http_path,protocol,http_host_name,cache_proxy[el_cache].EXPIRE );
			else if (strcmp(cache_proxy[el_cache].EXPIRE ,"")==0 && strcmp(cache_proxy[el_cache].LAST_MOD , "")==0)
				snprintf(condition_message, LENGTH_MAX, "%s %s %s \r \n HOST : %s \r \n USER-AGENT: HTTPTool/1.0 \r\n If-MODIFIED_SINCE: %s \r\n\r\n", method, http_path,protocol,http_host_name,cache_proxy[el_cache].LAST_ACCESS_DATE);
			else if (strcmp(cache_proxy[el_cache].EXPIRE, "") == 0)
				snprintf(condition_message, LENGTH_MAX, "%s %s %s \r \n HOST : %s \r \n USER-AGENT: HTTPTool/1.0 \r\n If-MODIFIED_SINCE: %s \r\n\r\n", method, http_path,protocol,http_host_name,cache_proxy[el_cache].LAST_MOD);
			else if(strcmp(cache_proxy[el_cache].LAST_MOD, "")==0)
				snprintf(condition_message, LENGTH_MAX, "%s %s %s \r \n HOST : %s \r \n USER-AGENT: HTTPTool/1.0 \r\n If-MODIFIED_SINCE: %s \r\n\r\n", method, http_path,protocol,http_host_name,cache_proxy[el_cache].EXPIRE);
			printf("INFO: Conditional GET generated :%s\n",condition_message);
			write(socket_web_file_descriptor, condition_message, LENGTH_MAX);

			response = (char *)malloc(1000000);
			checksum = Extract_Read(socket_web_file_descriptor, response);

			to_client = (char*)malloc(strlen(response));
			if(strstr(response,"304 Not Modified")!= NULL){
				printf("'304 NOT MODIFIED' message is received. FILE IN CACHE IS SENT \n");
				memcpy(to_client, cache_proxy[el_cache].body, strlen(cache_proxy[el_cache].body));
				cache_update(URL_NAME, NULL, 0, el_cache);
			}
			else{
				printf("SERVER: FILE MODIFIED\n");
				memcpy(to_client, response, strlen(response));
				cache_update(URL_NAME, NULL, 0, el_cache);
				cache_proxy[-- number_of_cache_entries] = clear;
				cache_update(URL_NAME, response, 1, 0);
			}
		}
		else {
			printf("SERVER: URL requested is not in cache\n");
			memset(message_client_forward, 0, LENGTH_MAX);
			snprintf(message_client_forward, LENGTH_MAX, "%s %s %s \r\n HOST: %s\r\n USER-AGENT: HTTPTool/1.0 \r\n\r\n", method, http_path,protocol,http_host_name);
			printf("SERVER: Request generated : %s\n", message_client_forward);
			write(socket_web_file_descriptor, message_client_forward, LENGTH_MAX);
			response= (char*)malloc(100000);
			checksum = Extract_Read(socket_web_file_descriptor,response);
			to_client = (char *)malloc(strlen(response));
			memcpy(to_client, response, strlen(response));
			cache_update(URL_NAME, response, 1,0);
		}
	}
	display_cache();
	write(client_file_descriptor, to_client, strlen(to_client)+1);
}
int Extract_Read(int file_descriptor, char *MESSAGE){
	int total_count = 0;
	char buff [LENGTH_MAX]={0};
	int message_count = 1;
	int h;
	while(message_count > 0){
		memset(buff, 0, sizeof(buff));
		message_count = read(file_descriptor, buff, LENGTH_MAX);
		if(message_count == 0)
			break;
		strcat(MESSAGE, buff);
		total_count = total_count + message_count;
		if(buff[message_count - 1] == EOF){
			strncpy(MESSAGE, MESSAGE, (strlen(MESSAGE)-1));
			total_count -- ;
			break;
		}
	}
	return total_count;
}

int main (int argc, char *argv[]){
	int socket_file_descriptor, communicate_file_descriptor, bind_file_descriptor, listen_file_descriptor;
	int port_no;
	struct sockaddr_storage remote_address;
	socklen_t addrlen;

	if(argc !=3){
		system_error("Incorrect calling. CORRECT USAGE: ./proxy <IPAddress_server> <Port Number>");
		return 0;
	}
	port_no = atoi(argv[2]);
	struct sockaddr_in server_address;
	socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

	if(socket_file_descriptor < 0)
		system_error("ERROR: In socket");

	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(argv[1]);
	server_address.sin_port = htons(port_no);

	bind_file_descriptor = bind(socket_file_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));

	if(bind_file_descriptor < 0)
		system_error("ERROR: Bind error");
	listen_file_descriptor = listen(socket_file_descriptor, 10);
	if(listen_file_descriptor < 0)
		system_error("ERROR : Listen error");
	memset(cache_proxy, 0, CACHE_MAX*sizeof(struct http_cache));

	addrlen = sizeof(remote_address);
	pthread_t x;
	printf("\n PROXY SERVER STATUS : ONLINE\n");
	while(1)
	{
		communicate_file_descriptor = accept(socket_file_descriptor, (struct sockaddr*)&remote_address, &addrlen);
		pthread_create(&x, NULL, (void*)(&server_proxy),(void*)(intptr_t)communicate_file_descriptor);
	}
}