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

	char pname[10], cname[10], address[25];
	while(1){
		printf("1. Register content\n2. Download content\n3. List content\n4. Deregister content\n5. Quit\n");
		read(0, str, 100);
		select = atoi(str);

		switch(select){
			case 1:
				printf("Name:\n");
				read(0, pname, 10);
				printf("Content Name:\n");
				read(0, cname, 10);
				printf("Address:\n");
				read(0, address, 25);

				struct pdu spdu;
				spdu.type = 'R';
				sprintf(spdu.data, "%s%s%s", pname, cname, address);
				write(s, &spdu, 100);
				break;
			case 5:
				exit(0);
			default:
				printf("Invalid input\n");
		}
	}


	exit(0);
}
