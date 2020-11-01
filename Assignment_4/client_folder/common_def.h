/*****common_def.h file*/
// Author: Sanket Agarwal and Dhiraj Kudva 
// Organisation: Texas A&M University. 
// Description:  containst the parser and the system error function. 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_MAX 10
#define LENGTH_MAX 1024

int parser_proxy(const char* sub_hdr, char* main_buf, char* final_op){
	
	char*string_sub =  strstr(main_buf, sub_hdr);
	if(!string_sub) { 
		return 0;
	}
	
	char *end_sub = strstr(string_sub, "\r\n");
	
	string_sub += strlen(sub_hdr);
	while(*string_sub == ' ') 
		++string_sub;
	while(*(end_sub - 1) == ' ') 
		--end_sub;
	strncpy(final_op, string_sub, end_sub - string_sub);// copy to the final op
	final_op[end_sub - string_sub] = '\0'; // eos is requirede. 
	return 1;

}
//this function is used to parser the input url and separate it as explained in the recitation sessions. 
int parser_client(char* URL, char *host_name, int *port_change, char *parse_path){
	char *token_url;
	char *tempH, *tempP, *t1, *t2;
	char strings[16];//changed by dhiraj : added [16]
	
	if (strstr(URL,"http") != NULL){
		token_url = strtok(URL, ":");
		t1 = token_url + 7;
	}
	else{
		t1 = URL;
	}
	t2 = (char*)malloc (64*sizeof(char));
	memcpy(t2, t1, 64);
	
	 if(strstr(t1, ":") != NULL){
    tempH = strtok(t1, ":");
    *port_change = atoi(t1 + strlen(tempH) + 1);
    sprintf(strings, "%d", *port_change);
    tempP = t1 + strlen(tempH) + strlen(strings) + 1;
  }
  else{
    tempH = strtok(t1, "/");   
    *port_change = 80;
    tempP = t2 + strlen(tempH);
  }
  if (strcmp(tempP, "") == 0)
    strcpy(tempP, "/");
  memcpy(host_name, tempH, 64);
  memcpy(parse_path, tempP, 256);
  return(0);
  
}


int system_error(const char*error_string)
{
	perror(error_string);
	exit(1);
}
