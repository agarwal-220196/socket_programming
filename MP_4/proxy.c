
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ELEMENT_NOT_IN_CACHE -1
#define MAX_CACHE_ENTRY 10
#define MAX_LEN 1024
#define STALE_ENTRY 1

struct cache_content {
	char url[256];
	char last_modified[50];
	char access_date[50];
	char expires[50];
	char *body;
};

static const struct cache_content reset_cache_data;
int cache_entries = 0;

struct cache_content http_cache_proxy[MAX_CACHE_ENTRY];

void cache_table_update(char *url, char *buf, int flag, int x)
{
	int j;

	// New Entry
	if (flag == 1) {
		if (cache_entries==MAX_CACHE_ENTRY) {
			http_cache_proxy[0] = reset_cache_data;
			for (j = 0; j < MAX_CACHE_ENTRY; j++) {
				if (j+1 != MAX_CACHE_ENTRY)
					http_cache_proxy[j] = http_cache_proxy[j+1];
				else {
					bzero(&http_cache_proxy[j], sizeof(struct cache_content));
					memcpy(http_cache_proxy[j].url, url, 256);
					http_cache_proxy[j].body = (char *)malloc(strlen(buf) * sizeof(char));
					memcpy(http_cache_proxy[j].body, buf, strlen(buf));
					parser_server("expires:", buf, http_cache_proxy[j].expires);
					parser_server("Last-Modified:", buf, http_cache_proxy[j].last_modified);
					parser_server("Date:", buf, http_cache_proxy[j].access_date);
				}
			}
		} else {
			http_cache_proxy[cache_entries] = reset_cache_data;
			memcpy(http_cache_proxy[cache_entries].url, url, 256);
			parser_server("expires:", buf, http_cache_proxy[cache_entries].expires);
			parser_server("Last-Modified:", buf, http_cache_proxy[cache_entries].last_modified);
			parser_server("Date:", buf, http_cache_proxy[cache_entries].access_date);
			http_cache_proxy[cache_entries].body = (char *)malloc(strlen(buf) * sizeof(char));
			memcpy(http_cache_proxy[cache_entries].body, buf, strlen(buf));
			cache_entries++;
		}
	} else {
		struct cache_content tmp;
    		struct tm tmp_t;
		time_t nw = time(NULL);
		const char* op_tmp = "%a, %d %b %Y %H:%M:%S GMT";
		bzero(&tmp, sizeof(struct cache_content));
		tmp = http_cache_proxy[x];
		for (j = x; j < cache_entries; j++) {
			if (j == cache_entries - 1)
				break;
			http_cache_proxy[j] = http_cache_proxy[j+1];
		}
		http_cache_proxy[cache_entries -1] = tmp;
		tmp_t = *gmtime(&nw);
		strftime(http_cache_proxy[cache_entries - 1].access_date, 50, op_tmp, &tmp_t);
	}
}

void display_cache_entries()
{
	int t;

	if (cache_entries == 0)
		fprintf(stdout, "cache is unoccupied currently\n");
	else {
		fprintf(stdout, "cache count: %d\n", cache_entries);
		for (t = 0; t < cache_entries; t++) {
			if (strcmp(http_cache_proxy[t].expires, "") != 0 &&
			    strcmp(http_cache_proxy[t].last_modified, "") != 0)
				fprintf(stdout, "index: %d  |  url: %s  |  access date: %s  |  expires: %s  |  last modified: %s\n\n",
			    		t, http_cache_proxy[t].url, http_cache_proxy[t].access_date, http_cache_proxy[t].expires,
					http_cache_proxy[t].last_modified);
			else if (strcmp(http_cache_proxy[t].expires, "") == 0 &&
				 strcmp(http_cache_proxy[t].last_modified, "") == 0)
				fprintf(stdout, "index: %d  |  url: %s  |  access Date: %s  |  expires: N/A  |  last modified: N/A\n\n",
					t, http_cache_proxy[t].url, http_cache_proxy[t].access_date);
			else if (strcmp(http_cache_proxy[t].expires, "") == 0)
				fprintf(stdout, "index: %d  |  url: %s  |  access date: %s  |  expires: N/A  |  last modified: %s\n\n",
					t, http_cache_proxy[t].url, http_cache_proxy[t].access_date, http_cache_proxy[t].last_modified);
			else if (strcmp(http_cache_proxy[t].last_modified, "") == 0)
				fprintf(stdout, "Index: %d  |  url: %s  |  Access Date: %s  |  expires: %s  |  last_modified: N/A\n\n",
					t, http_cache_proxy[t].url, http_cache_proxy[t].access_date, http_cache_proxy[t].expires);
		}
	}
}


int is_it_fresh(int cache_pointer)
{    
	struct tm tmp_t, expires;
	time_t nw  = time(NULL);
	tmp_t = *gmtime(&nw);

	if (strcmp(http_cache_proxy[cache_pointer].expires, "") != 0) {
		strptime(http_cache_proxy[cache_pointer].expires, "%a, %d %b %Y %H:%M:%S %Z", &expires);
		time_t EXP = mktime(&expires);
		time_t NOW = mktime(&tmp_t);
		if (difftime (NOW, EXP) < 0)
			return STALE_ENTRY;
		else
			return -1;
	} else
		return -1;
}

int element_of_cache_table(char *url)
{
	int b;

	for (b = 0; b < MAX_CACHE_ENTRY; b++) {
		if (strcmp(http_cache_proxy[b].url, url)==0)
			return b;
	}

	return ELEMENT_NOT_IN_CACHE;
}

int socket_web(char *host)
{
	struct addrinfo dynamic_address, *ai, *p;
	int ret_val, server_socket_descriptor;

	bzero(&dynamic_address, sizeof(dynamic_address));
	dynamic_address.ai_family = AF_INET;
	dynamic_address.ai_socktype = SOCK_STREAM;
	ret_val = getaddrinfo(host, "http", &dynamic_address, &ai);
	if (ret_val != 0) {
		fprintf(stderr, "SERVER: %s\n", gai_strerror(ret_val));
		exit(1);
	}
	for (p = ai; p != NULL; p = p->ai_next) {
		server_socket_descriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (server_socket_descriptor >= 0 && (connect(server_socket_descriptor, p->ai_addr, p->ai_addrlen) >= 0))
			break;
	}
	if (p == NULL)
		server_socket_descriptor = -1;

	freeaddrinfo(ai);
	return server_socket_descriptor;
}


int http_proxy_server(int client_fd)
{
	int server_socket_descriptor, return_read, cache_table_element, extract_read_return, port = 80;
	char *message_pointer;
	char client_message[MAX_LEN] = {0};
	char *read_extractor, *forward_client;
	char name_of_host[64], path_name[256], url[256], conditional_get_message[256], url_parse[256];
	char http_method[8];
	char http_protocol[16];

	message_pointer = (char *)malloc(MAX_LEN * sizeof(char));
	return_read = read(client_fd, message_pointer, MAX_LEN);
	printf("SERVER: Request retrieved from client: \n%s", message_pointer);
	if (return_read < 0)
		system_error ("SERVER: Error in extracting message request from client");
	sscanf(message_pointer, "%s %s %s", http_method, url, http_protocol);
	if ((cache_table_element = element_of_cache_table (url)) != ELEMENT_NOT_IN_CACHE
	     && (is_it_fresh(cache_table_element) == STALE_ENTRY)) {
		fprintf (stdout, "SERVER: Requested url: %s is in cache and is is_it_fresh\n", url);
		cache_table_update(url, NULL, 0, cache_table_element);
		forward_client = (char *)malloc(strlen(http_cache_proxy[cache_table_element].body) * sizeof(char));
		memcpy(forward_client, http_cache_proxy[cache_table_element].body, strlen(http_cache_proxy[cache_table_element].body));
	} else { // In cache, but stale
		bzero(name_of_host, 64);
		bzero(path_name, 256);
		memcpy(&url_parse[0], &url[0], 256);
		parse_client(url_parse, name_of_host, &port, path_name);
		if ((server_socket_descriptor = socket_web (name_of_host)) == -1)
			system_error ("SERVER: Error in connecting with web server");
		fprintf(stdout, "SERVER: Successfully connected to web server %d\n", server_socket_descriptor);
		if (cache_table_element != ELEMENT_NOT_IN_CACHE) { // cache entry expired
			fprintf(stdout, "SERVER: Requested url: %s is in cache but is expired\n", url);
			if (strcmp(http_cache_proxy[cache_table_element].expires, "") != 0 &&
		    	    strcmp(http_cache_proxy[cache_table_element].last_modified, "") != 0)
				snprintf(conditional_get_message, MAX_LEN,
					 "%s %s %s\r\nHost: %s\r\nUser-Agent: HTTPTool/1.0\r\nIf-Modified_Since: %s\r\n\r\n",
					http_method, path_name, http_protocol, name_of_host, http_cache_proxy[cache_table_element].expires);
			else if (strcmp(http_cache_proxy[cache_table_element].expires, "") == 0 &&
			 	 strcmp(http_cache_proxy[cache_table_element].last_modified, "") == 0)
				snprintf(conditional_get_message, MAX_LEN,
				 	 "%s %s %s\r\nHost: %s\r\nUser-Agent: HTTPTool/1.0\r\nIf-Modified_Since: %s\r\n\r\n",
				 	 http_method, path_name, http_protocol, name_of_host, http_cache_proxy[cache_table_element].access_date);
			else if (strcmp(http_cache_proxy[cache_table_element].expires, "") == 0)
				snprintf(conditional_get_message, MAX_LEN,
				 	 "%s %s %s\r\nHost: %s\r\nUser-Agent: HTTPTool/1.0\r\nIf-Modified_Since: %s\r\n\r\n",
				 	 http_method, path_name, http_protocol, name_of_host, http_cache_proxy[cache_table_element].last_modified);
			else if (strcmp(http_cache_proxy[cache_table_element].last_modified, "") == 0)
				snprintf(conditional_get_message, MAX_LEN,
				 	 "%s %s %s\r\nHost: %s\r\nUser-Agent: HTTPTool/1.0\r\nIf-Modified_Since: %s\r\n\r\n",
				 	 http_method, path_name, http_protocol, name_of_host, http_cache_proxy[cache_table_element].expires);
			fprintf(stdout, "Conditional GET Generated: \n%s", conditional_get_message);
			write(server_socket_descriptor, conditional_get_message, MAX_LEN);
			read_extractor = (char *)malloc(100000 * sizeof(char));
			extract_read_return = extract_read_from_main(server_socket_descriptor, read_extractor);
			forward_client = (char *)malloc(strlen(read_extractor) * sizeof(char));
			if (strstr(read_extractor, "304 Not Modified") != NULL) {
				fprintf(stdout, "'304 Not Modified' received. Sending file in cache\n");
				memcpy(forward_client, http_cache_proxy[cache_table_element].body, strlen(http_cache_proxy[cache_table_element].body));
				cache_table_update(url, NULL, 0, cache_table_element);
			} else {
				fprintf(stdout, "SERVER: File was modified\n");
				memcpy(forward_client, read_extractor, strlen(read_extractor));
				cache_table_update(url, NULL, 0, cache_table_element);
				http_cache_proxy[--cache_entries] = reset_cache_data;
				cache_table_update(url, read_extractor, 1, 0);
			}
		} else { // document is not cached
			fprintf(stdout, "SERVER: Requested url is not in cache\n");
			bzero(client_message, MAX_LEN);
			snprintf(client_message, MAX_LEN,
			 	 "%s %s %s\r\nHost: %s\r\nUser-Agent: HTTPTool/1.0\r\n\r\n",
			 	 http_method, path_name, http_protocol, name_of_host);
			fprintf(stdout, "SERVER: Request generated: \n%s", client_message);
			write(server_socket_descriptor, client_message, MAX_LEN);
			read_extractor = (char *)malloc(100000 * sizeof(char));
			extract_read_return = extract_read_from_main(server_socket_descriptor, read_extractor);
			forward_client = (char *)malloc(strlen(read_extractor) * sizeof(char));
			memcpy(forward_client, read_extractor, strlen(read_extractor)); 
			cache_table_update(url, read_extractor, 1, 0);
		}
	}
	display_cache_entries();
	write(client_fd, forward_client, strlen(forward_client) + 1);
}

int extract_read_from_main(int file_descriptor, char *ptr)
{ 
	int total_count = 0, cnt = 1;
	char buffer[MAX_LEN];

	while (cnt > 0) {
		bzero(buffer, sizeof(buffer));
		cnt = read(file_descriptor, buffer, MAX_LEN);
		if (cnt == 0)
			break;
		strcat(ptr, buffer);
		total_count += cnt;
		if (buffer[cnt - 1] == EOF) {
			strncpy(ptr,ptr,(strlen(ptr)-1));
			total_count--;
			break;
		}
	}

	return total_count;
}


int main(int argc, char *argv[])
{
	int socket_file_descriptor, connect_file_descriptor;
	int port_number;
	struct sockaddr_storage remote_address;
	struct sockaddr_in client_address;
	socklen_t length;
	pthread_t thread;

	if (argc != 3) {
		system_error ("USAGE: ./proxy <Server IP Address> <Port_Number>");
		return 0;
	}
  
	port_number = atoi(argv[2]);
	socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_file_descriptor < 0)
		system_error ("ERR: Socket Error");
	bzero( &client_address, sizeof(client_address));
	client_address.sin_family = AF_INET;
	client_address.sin_addr.s_addr = inet_addr(argv[1]);
	client_address.sin_port = htons(port_number);
	if (bind(socket_file_descriptor, (struct sockaddr *)&client_address, sizeof(client_address)) < 0)
		system_error ("ERR: Bind Error");
	if (listen(socket_file_descriptor, 10) < 0)
		system_error ("ERR: Listen Error");
	bzero(http_cache_proxy, MAX_CACHE_ENTRY * sizeof(struct cache_content));
	length = sizeof(remote_address);
	fprintf(stdout, "\nPROXY SERVER is online\n\n");
	while (1) {
		connect_file_descriptor = accept(socket_file_descriptor, (struct sockaddr*)&remote_address,&length);
		pthread_create(&thread, NULL, (void *)(&http_proxy_server), (void *)(intptr_t)connect_file_descriptor);
	}
}
