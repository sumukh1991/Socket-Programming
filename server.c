/****
Authors		: Halymysore Ravindra, Sumukh
Date		: 06/18/2016
Description	: Server code - Socket programming
****/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv){

	// Server side code 

	// Create socket -> Bind socket to address -> Listen -> 
	// -> Establish connection -> Read/Recv -> Write/Send -> Close

	// *** Create socket ***

	// IPv4 protocol family, stream type and default protocol (TCP in this case)
	int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

	if(socketDescriptor < 0){
		perror("Error creating socket");
		exit(1);
	}

	// *** Bind socket to address name ***

	struct sockaddr_in serverAddress;

	// Clear hte struct
	memset(&serverAddress, 0, sizeof(struct sockaddr_in));

	// IPv4 protocol family, random port, any address
	serverAddress.sin_family 		= AF_INET;
	serverAddress.sin_port 	 		= htons(1314);
	serverAddress.sin_addr.s_addr 	= INADDR_ANY;

	if(bind(socketDescriptor, (struct sockaddr *) &serverAddress, sizeof(struct sockaddr_in)) == -1){
		perror("Error binding socket to address name");
		exit(1);
	} 

	// *** Listen (Passive end) to connection requests ***
	if(listen(socketDescriptor, 3) == -1 ){
		perror("Error listening to connect requests");
		exit(1);
	}
	
	// *** Establish connection ***
	
	struct sockaddr_in clientAddress;

	int addrLen = sizeof(struct sockaddr_in);

	int connectionDescriptor = accept(socketDescriptor, (struct sockaddr *)&clientAddress, &addrLen);

	if(connectionDescriptor == -1){
		perror("Error accepting the request");
		exit(1);
	}

	// *** Read/Recv ***
	char buffer[200];

	memset(buffer,'\0',200);

	if (read(connectionDescriptor, buffer, 200) < 0){
		perror("Error receiving data");
		exit(1);
	}

	printf("Received msg from client %s (%d): %s\n", inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port, buffer);

	char *successMsg = "Received msg successfully";

	// Write/Send
	if(write(connectionDescriptor, successMsg, 30) < 0){
		perror("Error sending data");
		exit(1);
	}

	// Close connection
	close(connectionDescriptor);

	// Close socket
	close(socketDescriptor);

	return 0;
}