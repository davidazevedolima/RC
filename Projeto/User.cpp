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
#define USER 6
#define PASS 9
#define VC 5
#define FILE 25 
#define FILESIZE 11
#define LIST 550
#define ASPORT "58048"
#define FSPORT "59048"

char  ASIP[IP];
char* ASport = (char*) ASPORT;
char  FSIP[IP];
char* FSport = (char*) FSPORT;

int fdAS, fdFS;

struct addrinfo *resAS, *resFS;

char uid[USER];
char password[PASS];

int rid = 0;

int tid;

void parseArgs(int argc, char* argv[]);

int createTCPClient(bool AS);
void closeTCPClient(bool AS);

void sendMessage(int fd, char* message, size_t msgLength);
void receiveMessage(int fd, char* message, size_t msgLength);

void login(char* message); 
void requestOperation(char* message); 
void validate(char* message);
void listFiles(char* message);
void retrieveFile(char* message); 
void uploadFile(char* message); 
void deleteFile(char* message);
void removeUser(char* message);


int main(int argc, char* argv[]) {
    char message[MSG];
    char command[CMD];
    // ./user [-n ASIP] [-p ASport] [-m FSIP] [-q FSport]
    parseArgs(argc, argv); 
    
    fdAS = createTCPClient(true); // main connection with AS server

    srand((unsigned int) time(NULL)); // rng initialization

    while (fgets(message, MSG, stdin) != NULL) {

        sscanf(message, "%s", command);
        
        // login UID pass
        if (strcmp(command, "login") == 0) {
            login(message);
        }
        // req Fop [Fname]
        else if (strcmp(command, "req") == 0) {
            requestOperation(message);
        }
        // val VC
        else if (strcmp(command, "val") == 0) {
            validate(message);
        }
        // list
        //    l
        else if (strcmp(command, "list") == 0 || strcmp(command, "l") == 0) {
            fdFS = createTCPClient(false);
            listFiles(message);
            closeTCPClient(false); 
        }
        // retrieve filename 
        //        r filename
        else if (strcmp(command, "retrieve") == 0 || strcmp(command, "r") == 0) {
            fdFS = createTCPClient(false);
            retrieveFile(message);
            closeTCPClient(false); 
        }
        // upload filename
        //      u filename
        else if (strcmp(command, "upload") == 0 || strcmp(command, "u") == 0) {
            fdFS = createTCPClient(false);
            uploadFile(message);
            closeTCPClient(false); 
        }
        // delete filename
        //      d filename
        else if (strcmp(command, "delete") == 0 || strcmp(command, "d") == 0) {
            fdFS = createTCPClient(false);
            deleteFile(message);
            closeTCPClient(false); 
        }
        // remove filename
        //      r filename
        else if (strcmp(command, "remove") == 0 || strcmp(command, "x") == 0) {
            fdFS = createTCPClient(false);
            removeUser(message);
            closeTCPClient(false); 
        }
        // exit 
        else if (strcmp(command, "exit") == 0) {
            closeTCPClient(true); 
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
        std::cerr << "Error while getting address info." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (AS)
	    n = connect(fd, resAS->ai_addr, resAS->ai_addrlen);
    else 
        n = connect(fd, resFS->ai_addr, resFS->ai_addrlen);

	if (n == -1) {
        std::cerr << "Error while connecting to socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    return fd; 
}

void closeTCPClient(bool AS) {
    if (AS) {
        freeaddrinfo(resAS);
        close(fdAS);
    }
    else {
        freeaddrinfo(resFS);
        close(fdFS);
    }
}

void sendMessage(int fd, char* message, size_t length) {
    ssize_t nleft, nwritten;

    char* ptr = message;
    nleft = length;

    while(nleft > 0) {
        nwritten = write(fd, ptr, nleft);
        
        if(nwritten <= 0) {
            std::cerr << "Error while writing to socket." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        nleft -= nwritten;
        ptr += nwritten;
    }
}

void receiveMessage(int fd, char* message, size_t length) {
    ssize_t nleft, nread;

    char* ptr = message;
    nleft = length;

    while(nleft > 0) {
        nread = read(fd, ptr, nleft);
        if(nread == -1) {
            std::cerr << "Error while reading from socket." << std::endl;
            exit(EXIT_FAILURE);
        }
        else if(nread == 0)
            break;          //closed by peer
        nleft -= nread;
        ptr += nread;

        /* if (message[length - nleft - 1] == '\n')
            break; */
    }
    
    ptr[length - nleft] = '\0';
}

void login(char* message) {
    char logUser[USER], logPass[PASS];
    char status[10];

    // login UID pass
    sscanf(message, "login %s %s", logUser, logPass);

    // LOG UID pass
    sprintf(message, "LOG %s %s\n", logUser, logPass);
    sendMessage(fdAS, message, strlen(message));

    memset(message, '\0', sizeof(message)); 

    // RLO status    
    receiveMessage(fdAS, message, MSG);
    sscanf(message, "RLO %s\n", status); 
    
    if (strcmp(status, "OK") == 0) {
        strcpy(uid, logUser);
        strcpy(password, logPass); 
        std::cout << "You are now logged in." << std::endl;
    }
    else if (strcmp(status, "NOK") == 0)
        std::cout << "Password is incorrect." << std::endl;
    else if (strcmp(status, "ERR") == 0)
        std::cout << "User " << uid << " is not registered." << std::endl;
}

void requestOperation(char* message) {
    char fileOp;
    char fileName[FILE];
    char status[10];

    // req Fop Filename 
    sscanf(message, "req %c %s", fileOp, fileName);
    
    // generate RID for the requested operation
    rid = generateRID(); 

    // REQ UID RID Fop [Fname]
    if (fileOp == 'R' || fileOp == 'U' || fileOp == 'D')
        sprintf(message, "REQ %s %d %s %s\n", uid, rid, fileOp, fileName);
    else
        sprintf(message, "REQ %s %d %s\n", uid, rid, fileOp);

    sendMessage(fdAS, message, strlen(message));
    memset(message, '\0', sizeof(message));

    // RRQ status    
    receiveMessage(fdAS, message, MSG);
    sscanf(message, "RRQ %s\n", status); 

    if (strcmp(status, "OK") == 0)
        std::cout << "Request accepted. Validation code sent to PD." << std::endl;
    else if (strcmp(status, "ELOG") == 0)
        std::cout << "Request denied. User: " << uid << " is not logged in." << std::endl;
    else if (strcmp(status, "EPD") == 0)
        std::cout << "Message could not be sent to the PD." << std::endl;
    else if (strcmp(status, "EUSER") == 0)
        std::cout << "Username " << uid << " is incorrect." << std::endl;
    else if (strcmp(status, "EFOP") == 0)
        std::cout << "File Operation " << fileOp << "is not valid." << std::endl;
    else if (strcmp(status, "ERR") == 0)
        std::cout << "Error in request." << std::endl;
}

// random integer within the range [1000, 9999]
int generateRID() {
    return 1000 + rand() % 9000;
}

void validate(char* message) {
    char valCode[VC];

    if (rid == 0) {
        std::cout << "No operation has been requested." << std::endl;
        return;
    }

    // val VC
    sscanf(message, "val %s", valCode);

    // AUT UID RID VC
    sprintf(message, "AUT %s %d %s\n", uid, rid, valCode); 
    sendMessage(fdAS, message, strlen(message));
    memset(message, '\0', sizeof(message));
    
    // RAU TID
    receiveMessage(fdAS, message, MSG); 
    sscanf(message, "RAU %d\n", tid); 
    
    if(tid == 0) {
        std::cout << "Authentication failed." << std::endl;
        return; 
    }
    else
        std::cout << "Authenticated! (TID=" << tid << ")" << std::endl;

    rid = 0;        // reset RID
}

void listFiles(char* message) {
    char status[10];

    // LST UID TID
    sprintf(message, "LST %s %d\n", uid, tid);
    
    sendMessage(fdFS, message, strlen(message)); 
    memset(message, '\0', sizeof(message));
    
    // RLS N[ Fname Fsize]*
    receiveMessage(fdFS, message, LIST);

    if (strcmp(message, "RLS EOF\n") == 0)
        std::cout << "No files are available." << std::endl;
    else if (strcmp(message, "RLS INV\n") == 0)
        std::cout << "AS validation error." << std::endl;
    else if (strcmp(message, "RLS ERR\n") == 0)
        std::cout << "LST request not properly formulated." << std::endl;
    else {
        displayList(message);
    }
}

void displayList(char* message) {
    int listSize;
    char* ptr = message;
    char fileName[FILE];
    char fileSize[FILESIZE];

    // RLS N
    sscanf(message, "RLS %d", &listSize);

    // [ Fname Fsize]*
    for (int i = 1; i <= listSize; i++) {
        // skip to next file information
        do {
            ptr++;
        } while(ptr[0] != ' ');
        do {
            ptr++;
        } while(ptr[0] != ' ');
        
        sscanf(ptr, "%s %s", fileName, fileSize); 
        std::cout << i << ". " << fileName << " " << fileSize << "B." << std::endl;  
    }
}
