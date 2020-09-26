/**************************common_def.h************************************/
//Author:Sanket Agarwal & Dhiraj Kudva (agarwal.220196, dhirajkudva)@tamu.edu
//Organisation: Texas A&M University
//Description: Simple Broadcast Chat Server structure files. 

#ifndef common_def
#define common_def

struct simple_broadcast_chat_server_header{

	unsigned int version:9; //9 bits as defined in the mannual. 
	unsigned int type:   7; //7 bits as defined. 
	int length;
};

struct simple_broadcast_chat_server_attribute{

	int type;
	int length;
	char payload_data [512];
	
};

struct simple_broadcast_chat_server_message{

	struct simple_broadcast_chat_server_header header;
	struct simple_broadcast_chat_server_attribute attribute[2];// to identify the two different msgs. i.e. the actual payload message and the username.
};


struct simple_broadcast_chat_server_client_user_information{

	char username[16];//as definedin the manual 
	int file_descriptor; 
	int client_count;
};



#endif 
int system_error(const char* error_string) //display and exit 
{
	error(error_string);
	exit(1);
}
