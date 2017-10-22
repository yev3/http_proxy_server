#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <signal.h>
#include <iostream>

#define MAXCLIENTNUMBER 20
#define MAXDATASIZE 128

char buffer[MAXDATASIZE];

// Error message function for server
void error(const char *msg) {
	perror(msg);
	exit(1);
}

void cleanExit(int signal) {
	exit(0);
}

void handleClient(int sock);

std::string receiveString(int sockfd);

int getMessageLength(int sockfd);

int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno, listNum;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	// Bad Arguments
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided");
		exit(2);
	}

	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);

	// Open the socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("ERROR opening socket");
	
	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind to the socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	// Listen
	if ((listNum = listen(sockfd, MAXCLIENTNUMBER)) < 0)
		error("Listen failed");

	while (1) {
		// Block and wait for client connections
		if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) < 0)
			error("Accept failed");
		
		pid_t pid = fork();

		if (pid == 0) {
			// Child process
			handleClient(newsockfd);
		}
		else if (pid > 0) {
			// Parent process
			close(newsockfd);
		}
		else {
			// Fork failed
			error("Forked failed");
			exit(1);
		}
	}
	std::cout<< "LETS CLOSE THAT SOCKET" <<"/n";
	close(sockfd);

	return 0;
}

void handleClient(int clientfd) {
	memset(buffer, 0, MAXDATASIZE);
	
	// Get Username from client
	std::string user = receiveString(clientfd);
	
	// Redirect stdout to client fd
	if ((dup2(clientfd, STDOUT_FILENO) < 0)) {
		error("dup2() error");
	}
	
	// Redirect error to client fd
	if ((dup2(clientfd, STDERR_FILENO) < 0)) {
		error("dup2() error");
	}
	
	// We don't want to close clientfd if it is by chance equal to other file descriptors
	if (clientfd != STDOUT_FILENO && clientfd != STDOUT_FILENO)
		close(clientfd);

	// Call "finger username" and send info to client
	if (execl("/bin/finger", "finger", user.c_str(), NULL) == -1) {
		error("execl failed");
		exit(3);
	}
}

std::string receiveString(int sockfd) {
	int n = 0;
	int totalBytesRead = 0;
	std::string userName;

	// Get message length
	int dataLength = getMessageLength(sockfd);

	// Use data length to read until all the data is received
	while (totalBytesRead < dataLength) {
		bzero(buffer, MAXDATASIZE + 1);
		n = recv(sockfd, buffer, MAXDATASIZE, 0);
		if (n == 0) {
			shutdown(sockfd, 0);
			error("Client Disconnected");
			break;
		}
		if (n < 0) {
			error("ERROR reading from socket");
			break;
		}
		totalBytesRead += n;
		userName += buffer;
	}
	
	return userName;
}

int getMessageLength(int sockfd) {
	int size = 0;
	int n = 0;
	
	// Peek at socket until we have the length (4 bytes)
	// Note that this technique is good when the amount of data transfered
	// is relatively low.
	while (n < sizeof(int)) {
		n = recv(sockfd, &buffer, sizeof(int), MSG_PEEK);
		if (n == 0) {
			shutdown(sockfd, 0);
			error("Client Disconnected");
			break;
		}
		if (n < 0) {
			error("ERROR reading from socket");
			break;
		}
	}
	
	// Actually get length
	n = recv(sockfd, &size, sizeof(int), 0);
	if (n == 0) {
		shutdown(sockfd, 0);
		error("Client Disconnected");
	}
	if (n < 0) {
		error("ERROR reading from socket");
	}
	
	//Convert data length to host endian
	int dataLength = ntohl(size);
	
	return dataLength;
}