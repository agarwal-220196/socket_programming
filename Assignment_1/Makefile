# ***************************************** Make file code *********************
# Author:	Sanket Agarwal & Dhiraj Kudva (agarwal.220196, dhirajkudva)@tamu.edu
#Organisation:	Texas A&M University
#Description:	Machine problem 1, Compiles server and client source code.



#for compiling server.c
sample : echos.o echo.o


echos.o: server.c common_def.h 
	cc -c server.c -o echos.o

#for compiling client.c 

echo.o: client.c common_def.h
	cc -c client.c -o echo.o

#clean to discard previous .o files 
clean:
# 	$(RM) *.o
	rm -f sample *.o core



