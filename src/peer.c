#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <fcntl.h>

#define	BUFSIZE 64

#define	MSG		"Any Message \n"

struct pdu {
	char type;
	char data[100];
};

struct registered {
	char name[10];
	char contentName[10];
};

struct registered registeredContent[32];
int regSize = 0;

unsigned int searchName(char* name);
unsigned int searchContent(char* content);

int main(int argc, char **argv) {
	char	*host = "localhost";
	int	port = 3000;
	char	str[100];
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, n, type;	/* socket descriptor and socket type	*/


	switch (argc) {
	case 1:
		break;
	case 2:
		host = argv[1];
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "usage: client [host [port]]\n");
		exit(1);
	}

	memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;                                                                
        sin.sin_port = htons(port);
                                                                                        
    /* Map host name to IP address, allowing for dotted decimal */
        if ( phe = gethostbyname(host) ){
                memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
        }
        else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
                                                                                
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "Can't create socket \n");
	
                                                                                
    /* Connect the socket */
        if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "Can't connect to %s\n", host);

	(void) write(s, MSG, strlen(MSG));

	int select;
	int i, listSize;

	struct pdu spdu, rpdu;
	char pname[10], cname[10], address[25];
	while(1){
		printf("1. Register content\n2. Download content\n3. List content\n4. Deregister content\n5. Quit\n");
		read(0, str, 10);
		select = atoi(str);

		switch(select){
			case 1:
				int n;
				printf("Name:\n");
				n = read(0, pname, 10);
				pname[n - 1] = '\0';
				printf("Content Name:\n");
				n = read(0, cname, 10);
				cname[n - 1] = '\0';
				printf("Address:\n");
				n = read(0, address, 25);
				address[n - 1] = '\0';

				spdu.type = 'R';
				strcpy(spdu.data, pname);
				strcpy(spdu.data+10, cname);
				strcpy(spdu.data+20, address);
				write(s, &spdu, sizeof(struct pdu));

				read(s, (struct pdu*)&rpdu, sizeof(struct pdu));
				if (rpdu.type == 'A'){
					struct registered reg;
					strcpy(reg.name, pname);
					strcpy(reg.contentName, cname);
					registeredContent[regSize] = reg;
					regSize++;
				} else {
					printf("Error Registering");
				}
				
				break;
			case 3:
				spdu.type = 'O';
				spdu.data[0] = '\0';
				write(s, &spdu, sizeof(struct pdu));

				read(s, (struct pdu*)&rpdu, sizeof(struct pdu));
				if (rpdu.type != 'O'){
					printf("Error");
					break;
				}
				listSize = atoi(rpdu.data);
				for (i = 0; i < listSize; i++){
					read(s, (struct pdu*)&rpdu, sizeof(struct pdu));
					printf("%s\n", rpdu.data);
				}
				
				break;
			case 4:
				spdu.type = 'T';

				printf("Name:\n");
				n = read(0, pname, 10);
				pname[n - 1] = '\0';
				printf("Content Name:\n");
				n = read(0, cname, 10);
				cname[n - 1] = '\0';

				
			case 5:
				exit(0);
			default:
				printf("Invalid input\n");
		}
	}


	exit(0);
}

unsigned int searchName(char* name){
	int i;
	unsigned int sum = 0;
	for (i = 0; i < regSize; i++){
		if (strcmp(name, registeredContent[i].name)){
			sum += 1 << i;
		}
	}
	return sum;
}

unsigned int searchContent(char* content){
	int i;
	unsigned int sum = 0;
	for (i = 0; i < regSize; i++){
		if (strcmp(content, registeredContent[i].contentName)){
			sum += 1 << i;
		}
	}
	return sum;
}
