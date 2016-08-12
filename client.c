/****
Authors		: Halymysore Ravindra, Sumukh
Date		: 06/18/2016
Description	: Client code - Socket programming
****/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){

	// Client side code 

	// Create Socket -> connect to server -> Send data -> Close

	// *** Create socket ***

	// IPv4 protocol family, stream type and default protocol (TCP in this case)
	int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

	if(socketDescriptor < 0){
		perror("Error creating socket");
		exit(1);
	}

	struct sockaddr_in serverAddress;

	// Clear the struct
	memset(&serverAddress, 0, sizeof(struct sockaddr_in));

	struct hostent *serverEnt;

	serverEnt = gethostbyname(argv[1]);

	// IPv4 protocol family, port supplied by user, localhost(127.0.0.1)
	serverAddress.sin_family 		= AF_INET;
	serverAddress.sin_port 	 		= htons(atoi(argv[2]));
	bcopy((char *)serverEnt->h_addr, (char *)&serverAddress.sin_addr.s_addr, serverEnt->h_length);
	
	if(connect(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) < 0){
		perror("Error connecting to server");
		exit(1);
	}

	// Write/Send data to server
	if(write(socketDescriptor, argv[3], 200) < 0){
		perror("Error sending data");
		exit(1);
	}

	char buffer[100];

	if(read(socketDescriptor, buffer, 100) < 0){
		perror("Error receiving data");
		exit(1);
	}

	printf("Message from server @host: %s\n", buffer);

	close(socketDescriptor);

}