#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <time.h>
#include <iostream>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define IP 20
#define MSG 500
#define CMD 10
#define ASPORT "58048"
#define FSPORT "59048"

char  ASIP[IP];
char* ASport = (char*) ASPORT;
char  FSIP[IP];
char* FSport = (char*) FSPORT;

int fdAS, fdFS;

struct addrinfo *resAS, *resFS;


void parseArgs(int argc, char* argv[]);

int createTCPClient(int fd, struct addrinfo *res);

void sendMessage(int fd, char* message, size_t msgLength);
void receiveMessage(int fd, char* message, size_t msgLength);

int main(int argc, char* argv[]) {
    char message[MSG];
    char command[CMD]; 
    // ./user [-n ASIP] [-p ASport] [-m FSIP] [-q FSport]
    parseArgs(argc, argv); 
    
    fdAS = createTCPClient(true); // main connection with AS server

    while (fgets(message, MSG, stdin) != NULL) {

        sscanf(message, "%s", command);
        
        // login UID pass
        if (strcmp(command, "login") == 0) {

        }
        // req Fop [Fname]
        else if (strcmp(command, "req") == 0) {

        }
        // val VC
        else if (strcmp(command, "val") == 0) {

        }
        // list
        //    l
        else if (strcmp(command, "list") == 0 || strcmp(command, "l") == 0) {

        }
        // retrieve filename 
        //        r filename
        else if (strcmp(command, "retrieve") == 0 || strcmp(command, "r") == 0) {

        }
        // upload filename
        //      u filename
        else if (strcmp(command, "upload") == 0 || strcmp(command, "u") == 0) {

        }
        // delete filename
        //      d filename
        else if (strcmp(command, "delete") == 0 || strcmp(command, "d") == 0) {

        }
        // remove filename
        //      r filename
        else if (strcmp(command, "remove") == 0 || strcmp(command, "x") == 0) {

        }
        // exit 
        else if (strcmp(command, "exit") == 0) {
            freeaddrinfo(resAS); 
            close(fdAS);
            break; 
        }
    }
    
    return 0;
}

void parseArgs(int argc, char* argv[]) {
    int opt; 

    while ((opt = getopt(argc, argv, "n:p:m:q:")) != -1) {
        switch (opt) {
            case 'n':
                strcpy(ASIP, optarg); 
                break;
            case 'p':
                ASport = optarg;
                break;
            case 'm':
                strcpy(FSIP, optarg);
                break;
            case 'q':
                FSport = optarg;
                break;
            default:
                break;
        }
    }

    if (strlen(ASIP) == 0)
        gethostname(ASIP, sizeof(ASIP)); 
    if (strlen(FSIP) == 0)
        gethostname(FSIP, sizeof(FSIP)); 
}


int createTCPClient(bool AS) {              // bool AS: true if connecting to AS, false if connecting to FS 
    int fd, errcode;
	ssize_t n;
	struct addrinfo hints;

	fd = socket(AF_INET, SOCK_STREAM, 0);	// TCP socket
	if (fd == -1) {
        std::cerr << "Error while creating socket." << std::endl;
        exit(EXIT_FAILURE);
    }

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;				// IPv4
	hints.ai_socktype = SOCK_STREAM;		// TCP socket

    if (AS)
	    errcode = getaddrinfo(ASIP, ASport, &hints, &resAS);
    else
        errcode = getaddrinfo(FSIP, FSport, &hints, &resFS);
	
    if (errcode != 0) {
        std::cerr << "Error while getting address info" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (AS)
	    n = connect(fd, resAS->ai_addr, resAS->ai_addrlen);
    else 
        n = connect(fd, resFS->ai_addr, resFS->ai_addrlen);

	if (n == -1) {
        std::cerr << "Error while connecting to socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    return fd; 
}

