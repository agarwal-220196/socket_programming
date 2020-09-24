//***************************************Common_definition.h****************************
//Author	: Sanket Agarwal & Dhiraj Kudva (agarwal.220196, dhirajkudva)@tamu.edu
//Organization	: Texas A&M University, Machine Problem 1 for ECEN 602
//Description	: Contains the definition of common functions such as readline() and written()


//including standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MESSAGE_LENGTH 120

static char efficient_read_buffer [MAX_MESSAGE_LENGTH];
static char *efficient_read_ptr;
static int read_count;

//***************************written() function definiton*******************/

int written (int socket_descriptor, char* buffer, int message_length)
{
	int write_check_value = 0;
	//loop until EINTR error is resolved
	write_checkpoint:
		//write  (file descriptor, starting buf address, number of bytes)
		write_check_value = write (socket_descriptor, buffer, (message_length+1));
	 	
		if(write_check_value < 0)
		{
			if (errno==EINTR)
				goto write_checkpoint; //complete the write if it was interrupted. 
			return (-1);		
		}
		else
			return(message_length);//complete message was written. 

}  

//******************readline() function definition with efficient_read ****************
int readline (int socket_descriptor, char * buffer, int max_message_length)
{
	char single_char;
	int eff_read_return, i;

	/*the main concept with efficient read is to read the complete buffer lenght the the read()
	and fill the buffer with one character at a time using a for loop.
	The efficient read will read the entire buffer in the first for loop call and resolve any
	EINTR errors. 
	
	In the next subsequent for loop calls, it will just return with the next character of the read 
	statement until the MAX_lengh. 
	the read line function would check if the character returned from the efficient read is a EOF
	character or any other error. 
	*/

	for (i=0; i<(max_message_length -1); i++)
	{
		eff_read_return = efficient_read(socket_descriptor, &single_char);
	
		if (eff_read_return == 1)
		{
			*buffer++ = single_char;
			if (single_char == '\n')
				break; // break the for loop at a new line character
		}
		else if (eff_read_return == 0)
		{
			*buffer = 0; //End Of File as described in the handout 
			return (i);
		}	
		else 
			return (-1);

	}
	bzero (efficient_read_buffer , (int) MAX_MESSAGE_LENGTH);
	read_count =0; 
	return (i+1); // including the null character that will be added.  
}


int efficient_read(int socket_descriptor, char * single_char)
{
	if (read_count<=0)//first time read
	{	
		efficient_read_checkpoint:
		   read_count = read (socket_descriptor, efficient_read_buffer , (int)MAX_MESSAGE_LENGTH);
		if (read_count < 0 )//partial read or other error 
		{	read_count-- ;//to incorporate for the extra read
			if (errno == EINTR)
				goto efficient_read_checkpoint;
			return (-1);		
		}
		else if (read_count==0)
			return (0);
		efficient_read_ptr = efficient_read_buffer ; //assiging to the first character. 
	}

	read_count-- ; //the last character would be '\n' and thus it will not return again. when \n is encounterd, the for loop and thus the call to efficient read, is stopped. 
	
	*single_char = *efficient_read_ptr++ ;// pass character by character. 
	return (1);

}

int system_error(const char * error_message)
{
	perror(error_message);
	exit(1);
}