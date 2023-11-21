#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

struct pdu {
	char type;
	char data[100];
};

struct content {
	char peerName[10];
	char contentName[10];
	char address[25];
};

struct content contents[32];
int contentsSize = 0;

//returns bit array of existing content in contents[]
unsigned int searchContent(char* content);
unsigned int searchName(char* name);

int main(int argc, char *argv[]) {
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	char	buf[100];		/* "input" buffer; any size > 0	*/
	char    *pts;
	int	sock;			/* server socket		*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
        int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;
	int	i;
                                                                                

	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "can't creat socket\n");
                                                                                
    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(s, 5);	
	alen = sizeof(fsin);

	struct pdu rpdu, spdu;
	while (1) {
		if (recvfrom(s, (struct pdu*)&rpdu, sizeof(struct pdu), 0, (struct sockaddr *)&fsin, &alen) < 0)
			fprintf(stderr, "recvfrom error\n");

		switch (rpdu.type){
			case 'R':
				struct content cont;

				strcpy(cont.peerName, rpdu.data);
				strcpy(cont.contentName, rpdu.data+10);
				strcpy(cont.address, rpdu.data+20);

				if (searchContent(cont.contentName)) {
					spdu.type = 'E';
					strcpy(spdu.data, "Content already exists");
					(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
					break;
				}

				contents[contentsSize] = cont;
				contentsSize++;

				spdu.type = 'A';
				spdu.data[0] = '\0';
				(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
				break;

			case 'O':
				spdu.type = 'O';
				sprintf(spdu.data, "%d", contentsSize);
				(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));

				for (i = 0; i < contentsSize; i++){
					sprintf(spdu.data, "name: %s\ncontent name: %s\n", contents[i].peerName, contents[i].contentName);
					(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
				}
				break;

			default:
		}

	}
}

unsigned int searchName(char* name){
	int i;
	unsigned int sum = 0;
	for (i = 0; i < contentsSize; i++){
		if (!strcmp(name, contents[i].peerName)){
			sum += 1 << i;
		}	
	}
	return sum;
}

unsigned int searchContent(char* content){
	int i;
	unsigned int sum = 0;
	for (i = 0; i < contentsSize; i++){
		if (!strcmp(content, contents[i].contentName)){
			sum += 1 << i;
		}	
	}
	return sum;
}
