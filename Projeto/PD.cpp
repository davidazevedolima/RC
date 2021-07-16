#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
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
#define USER 6
#define PASS 9
#define VC 5
#define FILE 25 
#define ASPORT "58048"
#define PDPORT "57048"

char  PDIP[IP];
char* PDport = (char*) PDPORT;
char  ASIP[IP];
char* ASport = (char*) ASPORT;

int fdClient, fdServer, fdStdin;

struct addrinfo *resClient, *resServer;

char uid[USER];
char password[PASS];

void parseArgs(int argc, char* argv[]);

int createUDPClient();
int createUDPServer();

void sendMessage(char* message, bool client);
void receiveMessage(char* message, bool client);

void registerUser(char* message);
bool validUser(char* user);
bool validPassword(char* password);

void validateCode();
char* fopName(char fileOp); 

bool exitAS(); 


int main(int argc, char* argv[]) {
    fd_set readfds, inputs; 
    int outfds;
    int maxfd;
 
    char message[MSG], command[CMD];

    // ./pd PDIP [-d PDport] [-n ASIP] [-p ASport]
    parseArgs(argc, argv);

    fdClient = createUDPClient();        // fd for PD (Client) <---> AS (Server)
    fdServer = createUDPServer();        // fd for AS (Client) <---> PD (Server)

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
                std::cerr << "Error while selecting file descriptors." << std::endl;
                exit(EXIT_FAILURE);

            default:

                if(FD_ISSET(fdServer, &readfds)) {
                    validateCode(); 
                }

                if (FD_ISSET(fdStdin, &readfds)) {
                    fgets(message, MSG, stdin);       // get line from stdin
                    sscanf(message, "%s", command);   // retrieve command
                
                    if (strcmp(command, "reg") == 0) {
                        registerUser(message);
                    }

                    else if (strcmp(command, "exit") == 0) {
                        if (strlen(uid) == 0) {		   // if user has not registered, then safely exit 
                            std::cout << "Exiting..." << std::endl;
                            exit(EXIT_SUCCESS);
                        }
                        else {
                            bool exitSuccess = exitAS(); 
                            if (exitSuccess) {
                                std::cout << "Exiting..." << std::endl;
                                exit(EXIT_SUCCESS);
                            }
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
int createUDPClient() {
    int fd, errcode;
    struct addrinfo hints;

    fd = socket(AF_INET, SOCK_DGRAM, 0);	// UDP socket
    if (fd == -1) {
        std::cerr << "Error while creating socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;				// IPv4
    hints.ai_socktype = SOCK_DGRAM;			// UDP socket

    errcode = getaddrinfo(ASIP, ASport, &hints, &resClient);
    if (errcode != 0) {
        std::cerr << "Error while getting address info." << std::endl;
        exit(EXIT_FAILURE);
    }
    
    return fd;
}

// AS (Client) <---> PD (Server)
int createUDPServer() {
    int fd, errcode;
    ssize_t n;
    struct addrinfo hints;

    fd = socket(AF_INET, SOCK_DGRAM, 0);	// UDP socket
    if (fd == -1) {
        std::cerr << "Error while creating socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;				// IPv4
    hints.ai_socktype = SOCK_DGRAM;			// UDP socket
    // PASSIVE: server (accepts connections)
    // NUMERICSERV: numeric port string
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    errcode = getaddrinfo(NULL, PDport, &hints, &resServer);
    if (errcode != 0) {
        std::cerr << "Error while getting address info" << std::endl;
        exit(EXIT_FAILURE);
    }

    n = bind(fd, resServer->ai_addr, resServer->ai_addrlen);
    if (n == -1) {
        std::cerr << "Error while binding server socket." << std::endl;
        exit(EXIT_FAILURE);
    }
    
    return fd;
}

void sendMessage(char* message, bool client) {
    int fd;
    struct addrinfo *res;
    ssize_t n;

    if (client) {
        fd = fdClient;
        res = resClient;
    } 
    else {  
        fd = fdServer; 
        res = resServer; 
    }

    n = sendto(fd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen);
	if (n == -1) {
        std::cerr << "Error while sending message." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void receiveMessage(char* message, bool client) {
    int fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    ssize_t n; 
    
    if (client) 
        fd = fdClient;
    else
        fd = fdServer;

	n = recvfrom(fd, message, 512, 0, (struct sockaddr*) &addr, &addrlen);
	if (n == -1) {
        std::cerr << "Error while receiving message." << std::endl;
        exit(EXIT_FAILURE);
    }
    message[n] = '\0';
}

void registerUser(char* message) {
    char regUser[50], regPass[50];
    char status[10];

    sscanf(message, "reg %s %s", regUser, regPass); 

    if (strlen(uid) != 0) {
        std::cout << "A user is already registered in this PD." << std::endl;
        return;
    }
    if (!validUser(regUser)) {
        std::cout << "Invalid user id." << std::endl;
        return;
    }
    if (!validPassword(regPass)) {
        std::cout << "Invalid password." << std::endl;
        return;
    }

    // REG UID pass PDIP PDport
    sprintf(message, "REG %s %s %s %s\n", regUser, regPass, PDIP, PDport);
    // Send message to AS as PD client 
    sendMessage(message, true);
    memset(message, '\0', sizeof(message));

    // RRG status
    receiveMessage(message, true); 
    sscanf(message, "RRG %s\n", status); 
    
    if (strcmp(status, "OK") == 0) {
        strcpy(uid, regUser);
        strcpy(password, regPass); 
        std::cout << "Registration successful." << std::endl;
    }
    else 
        std::cout << "Registration unsuccessful." << std::endl;
}

// 5-digit UID
bool validUser(char* user) {
    int length = strlen(user);
    if (length != USER - 1)
        return false;
    
    for (int i = 0; i < length; i++) {
        if(!isdigit(user[i]))    
            return false; 
    }
    return true;
}

// 8 alphanumerical character password
bool validPassword(char* password) {
    int length = strlen(password); 

    if (length != PASS - 1)
        return false;
    
    for (int i = 0; i < length; i++) {
        if (!isalnum(password[i]))
            return false; 
    }
    return true; 
}

void validateCode() {
    char message[MSG];
    char valUser[50];
    char valCode[VC];
    char fileOp;
    char fileName[FILE];

    // VLC UID VC Fop [Fname]
    receiveMessage(message, false); 
    sscanf(message, "VLC %s %s %c %s\n", valUser, valCode, &fileOp, fileName); 

    // VC=valCode, Fop: Fname
    std::cout << "VC=" << valCode << ", " << fopName(fileOp);
    if (fileOp == 'R' || fileOp == 'U' || fileOp == 'D')
        std::cout << ": " << fileName;
    std::cout << "." << std::endl;

    // RVC status
    if (strcmp(valUser, uid) != 0)
       sprintf(message, "RVC %s NOK\n", valUser);
    else
        sprintf(message, "RVC %s OK\n", uid);
        
    sendMessage(message, false);
}

char* fopName(char fileOp) {
    switch (fileOp) {
        case 'L':
            return (char*) "list";
        case 'R':  
            return (char*) "retrieve";
        case 'U':
            return (char*) "upload";
        case 'D':
            return (char*) "delete"; 
        case 'X':
            return (char*) "remove";
        default: 
            return (char*) "unknown"; 
    }
}

bool exitAS() {
    char message[MSG];
    char status[10];

    // UNR UID pass
    sprintf(message, "UNR %s %s\n", uid, password); 
    sendMessage(message, true); 

    memset(message, '\0', sizeof(message));

    // RUN status
    receiveMessage(message, true);
    sscanf(message, "RUN %s\n", status);
    
    if (strcmp(status, "OK") == 0) {
        std::cout << "Unregistration successful" << std::endl;

        // close sockets
        freeaddrinfo(resClient); 
        close(fdClient);
        freeaddrinfo(resServer);
        close(fdServer); 
        return true; 
    }
    else {
        std::cout << "Unregistration failed" << std::endl;
        return false; 
    }
}
