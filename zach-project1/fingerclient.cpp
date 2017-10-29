#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>

#define MAXDATASIZE 12

char buffer[MAXDATASIZE];

// Error message function
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void sendString(int sockfd, char * stringToSend);

std::string receiveString(int sockfd);

int main(int argc, char *argv[]) {
	int sockfd, rv;
	struct addrinfo hints, *serverInfo, *p;
	memset(&buffer, 0, MAXDATASIZE);
	
	// Bad Arguments
	if (argc < 1) {
		fprintf(stderr, "ERROR, usage username@hostname:server_port");
		exit(1);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	// Parse arguments
	char * userName = strtok(argv[1],"@");
	char * hostName = strtok(NULL,":");
	char * portNumber = strtok(NULL,"/n");

	if ((rv = getaddrinfo(hostName, portNumber, &hints, &serverInfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	for (p = serverInfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("connect");
			close(sockfd);
			continue;
		}

		// Free memory
		freeaddrinfo(serverInfo);
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	
	// Send username argument to server
	sendString(sockfd, userName);
	
	// Receive information about the user from server
	std::string userInfo = receiveString(sockfd);


	// Print out info
	std::cout << userInfo;


	// Close socket
	close(sockfd);

	return 0;
}

void sendString(int sockfd, char * stringToSend) {
	int n = 0;
	int numSent = 0;
	int nameSize = strlen(stringToSend);
	
	// Convert argument to correct endian
	int nameLength = htonl(nameSize);
	
	// Add length to buffer
	memcpy(buffer, &nameLength, sizeof(int));
	
	// Add name to buffer
	char * newBuffer = buffer + sizeof(int);
	n = sprintf(newBuffer, stringToSend);
	
	int dataLength = nameSize * sizeof(char) + sizeof(int);
	
	// Keep sending until all the data in the buffer is sent
	newBuffer = buffer;
	while (numSent < nameSize) {
		n = send(sockfd, newBuffer, dataLength, 0);
		if (n == 0) {
			error("Server Disconnected");
			close(sockfd);
		}
		else if (n < 0)
			error("ERROR writing to socket");
		else {
			newBuffer += n;
			dataLength -= n;
			numSent += n;
		}
	}
}

std::string receiveString(int sockfd) {
	int n = 0;
	std::string userInfo;
	
	// Receive until no more data is sent
	while (1) {
	bzero(buffer, MAXDATASIZE + 1);
	n = recv(sockfd, buffer, MAXDATASIZE, 0);
	if (n == 0) {
		break;
	}
	if (n < 0) {
		error("ERROR reading from socket");
		break;
	}
	userInfo += buffer;
	}
	
	return userInfo;
}