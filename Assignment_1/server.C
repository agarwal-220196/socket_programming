//***************************************server.C****************************
//Author        : Sanket Agarwal & Dhiraj Kudva (agarwal.220196, dhirajkudva)@tamu.edu
//Organization  : Texas A&M University, Machine Problem 1 for ECEN 602
/*Description   : Contains the definition of server that takes the port number from the command line
		  to bind, listen and accept different IPv4 connections to that port number. It
                  listens to different connections and echos back the same message recevied by the 
                  client*/ 

//including standard libraries & common_def.h
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <common_def.h>


