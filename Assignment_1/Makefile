# ***************************************** Make file code *********************
# Author: Sanket Agarwal & Dhiraj Kudva (agarwal.220196, dhirajkudva)@tamu.edu
#Organisation: Texas A&M University
#Description: Machine problem 1, Compiles server and client source code.



#for compiling server.c
sample : echos.o echo.o

<<<<<<< HEAD

echos.o: server.c common_def.h 
	cc -c server.c -o echos.o
=======
echos.o: server.c common_def.h
	gcc -I . server.c -o echos
>>>>>>> dbbfb0955317ae862a9f57e764d9a7d4caf476cd

#for compiling client.c

echo.o: client.c common_def.h
<<<<<<< HEAD
	cc -c client.c -o echo.o
=======
	gcc -I . client.c -o echo
>>>>>>> dbbfb0955317ae862a9f57e764d9a7d4caf476cd

#clean to discard previous .o files
clean:
<<<<<<< HEAD
# 	$(RM) *.o
	rm -f sample *.o core



=======
	$(RM) echo*
>>>>>>> dbbfb0955317ae862a9f57e764d9a7d4caf476cd
