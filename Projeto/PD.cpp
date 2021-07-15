#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define IPLENGTH 20 

#define ASPORT "58048"
#define PDPORT "57048"

char  PDIP[IPLENGTH];
char* PDport = (char*) PDPORT;
char  ASIP[IPLENGTH];
char* ASport = (char*) ASPORT;

// ./pd PDIP [-d PDport] [-n ASIP] [-p ASport]

void parseArgs(int argc, char* argv[]);
int establishASConnection(); 
void sendASmessage(char* message);
void receiveASmessage(char* message);


int main(int argc, char* argv[]) {
    parseArgs(argc, argv);

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
