#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <algorithm>
#include <iostream>

#define IP 20 
#define MSG 500
#define CMD 10
#define ASPORT "58048"
#define PDPORT "57048"

char  PDIP[IP];
char* PDport = (char*) PDPORT;
char  ASIP[IP];
char* ASport = (char*) ASPORT;

int fdClient, fdServer, fdStdin;

struct addrinfo *resClient, *resServer;
struct sockaddr_in clientAddr, serverAddr; 
socklen_t serverAddrlen; 
socklen_t clientAddrlen; 

void parseArgs(int argc, char* argv[]);
int establishASConnection(); 
int createPDClient();
int createPDServer();
void sendASmessage(char* message);
void receiveASmessage(char* message);

void validateCode(); 


int main(int argc, char* argv[]) {
	fd_set readfds, inputs; 
	int outfds;
    int maxfd;

    char message[MSG], command[CMD];

    // ./pd PDIP [-d PDport] [-n ASIP] [-p ASport]
	parseArgs(argc, argv);

    fdClient = createPDClient();        // fd for PD (Client) <---> AS (Server)
    fdServer = createPDServer();        // fd for AS (Client) <---> PD (Server)

	fdStdin = STDIN_FILENO;             // fd for stdin (0)

    maxfd = std::max(fdClient, fdServer);

	FD_ZERO(&inputs); 
	FD_SET(fdServer, &inputs);
	FD_SET(fdStdin, &inputs); 

	while (true) {
		readfds = inputs;
		
		outfds = select(maxfd + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, NULL); 
		
		switch(outfds) {

			case 0:
                // timeout event
				break;
                
			case -1:
				exit(EXIT_FAILURE);
                
            default:

				if(FD_ISSET(fdServer, &readfds)) {
					validateCode(); 
				}

                if (FD_ISSET(fdStdin, &readfds)) {
                    fgets(message, MSG, stdin);         // get line from stdin
                    sscanf(message, "%s", command);     // retrieve command
                
					if (strcmp(command, "reg") == 0) {
						registerUser(message);
					}

                    else if (strcmp(command, "exit") == 0) {
                        if (strlen(uid) == 0) 		   // if user has not registered, then safely exit 
                            exit(EXIT_SUCCESS);
                        else {
                            bool exitSuccess = exitAS();
                            if (exitSuccess)
                                exit(EXIT_SUCCESS);
                        }
                        
                    }
				}
		}
		

	}
	return 0;
}

void parseArgs(int argc, char* argv[]) {
	int opt; 

    strcpy(PDIP, argv[1]);
	while ((opt = getopt(argc, argv, "d:n:p:")) != -1) {
        switch (opt) {
            case 'd':
                PDport = optarg;
                break;
            case 'n':
				strcpy(ASIP, optarg);
                break;
            case 'p':
				ASport = optarg; 
                break;
            default:
                break;
        }
	}

	if (strlen(PDIP) == 0)
		gethostname(PDIP, sizeof(PDIP)); 
	if (strlen(ASIP) == 0)
		gethostname(ASIP, sizeof(ASIP)); 
}

// PD (Client) <---> AS (Server)
int createPDClient() {
	int fd, errcode;
	struct addrinfo hints;

	fd = socket(AF_INET, SOCK_DGRAM, 0);	// UDP socket
	if (fd == -1) /* error */ exit(1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;				// IPv4
	hints.ai_socktype = SOCK_DGRAM;			// UDP socket

	errcode = getaddrinfo(ASIP, ASport, &hints, &resClient);
	if (errcode != 0) /* error */
		exit(EXIT_FAILURE);
	
    return fd;
}

// AS (Client) <---> PD (Server)
int createPDServer() {
    int fd, errcode;
	ssize_t n;
	struct addrinfo hints;

	fd = socket(AF_INET, SOCK_DGRAM, 0);	// UDP socket
	if (fd == -1) /* error */ exit(1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;				// IPv4
	hints.ai_socktype = SOCK_DGRAM;			// UDP socket
    // PASSIVE: server (accepts connections)
    // NUMERICSERV: numeric port string
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

	errcode = getaddrinfo(NULL, PDport, &hints, &resServer);
	if (errcode != 0) 
		exit(EXIT_FAILURE);

	n = bind(fd, resServer->ai_addr, resServer->ai_addrlen);
	if (n == -1) 
		exit(EXIT_FAILURE);
	
	return fd;
}
