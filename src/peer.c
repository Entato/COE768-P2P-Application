#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h> 
#include <sys/signal.h>
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

uint32_t searchName(char* name);
uint32_t searchContent(char* content);
int getIndex(uint32_t bitArray);

int main(int argc, char **argv) {
	char	*host = "localhost";
	int	port = 3000;
	char	str[100];
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, n, type;	/* socket descriptor and socket type	*/
	int	sd;


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
	int search, index;

	char downloadPort[6];
	int alen;
	struct sockaddr_in server;
	struct hostent *hp;

	struct pdu spdu, rpdu;
	char pname[10], cname[10], address[20];
	while(1){
		printf("1. Register content\n2. Download content\n3. List content\n4. Deregister content\n5. Quit\n");
		read(0, str, 10);
		select = atoi(str);

		switch(select){
			case 1:
				printf("Name:\n");
				n = read(0, pname, 10);
				pname[n - 1] = '\0';
				printf("Content Name:\n");
				n = read(0, cname, 10);
				cname[n - 1] = '\0';

				int contentFile = open(cname, O_RDONLY);

				if (contentFile == -1){
					printf("File does not exist\n");
					break;
				}

				//Creating TCP socket
				sd = socket(AF_INET, SOCK_STREAM, 0);
				bzero((char*)&server, sizeof(struct sockaddr_in));
				server.sin_family = AF_INET;
				server.sin_port = htons(0);
				server.sin_addr.s_addr = htonl(INADDR_ANY);
				if(bind(sd, (struct sockaddr*)&server, sizeof(server)) == -1){
					fprintf(stderr, "Can't bind name to socket");
				}

				alen = sizeof(struct sockaddr_in);
				getsockname(sd, (struct sockaddr*) &server, &alen);

				spdu.type = 'R';
				strcpy(spdu.data, pname);
				strcpy(spdu.data+10, cname);
				sprintf(spdu.data+20, "%d", htons(server.sin_port));
				write(s, &spdu, sizeof(struct pdu));

				read(s, (struct pdu*)&rpdu, sizeof(struct pdu));
				if (rpdu.type == 'A'){
					struct registered reg;
					strcpy(reg.name, pname);
					strcpy(reg.contentName, cname);
					registeredContent[regSize] = reg;
					regSize++;

					switch (fork()){
						//listening server
						case 0:
							listen(sd, 5);
							(void) signal(SIGCHLD, SIG_IGN);

							struct sockaddr_in client;
							int new_sd;
							int client_length = sizeof(client);
							while(1){
								new_sd = accept(sd, (struct sockaddr*)&client, &client_length);

								switch(fork()){
									case 0:
										close(sd);
										
										read(new_sd, &rpdu, sizeof(struct pdu));
										spdu.type = 'C';
										bzero((char*)&spdu.data, sizeof(spdu.data));
										while((i = read(contentFile, spdu.data, 100)) > 0){
											printf("%s", spdu.data);
											write(new_sd, &spdu, sizeof(struct pdu));
											bzero((char*)&spdu.data, sizeof(spdu.data));
										}
										spdu.type = 'A';
										write(new_sd, &spdu, sizeof(struct pdu));
										close(new_sd);
										exit(0);
									default:
										close(new_sd);
								}
							}
							break;
						default:
							printf("Listening server created\n");
							break;
						case -1:
							fprintf(stderr, "fork error\n");
					}
				} else if (rpdu.type == 'E'){
					printf("%s\n", rpdu.data);
				}
				
				break;
			case 2:
				printf("Content Name:\n");
				n = read(0, cname, 10);
				cname[n - 1] = '\0';

				spdu.type = 'S';
				strcpy(spdu.data, cname);
				write(s, &spdu, sizeof(struct pdu));

				read(s, (struct pdu*)&rpdu, sizeof(struct pdu));
				if (rpdu.type == 'S'){
					strcpy(address, rpdu.data);
					strcpy(downloadPort, rpdu.data+20);

					port = atoi(downloadPort);

					if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
						fprintf(stderr, "Can't create socket\n");
					}

					bzero((char*)&server, sizeof(struct sockaddr_in));
					server.sin_family = AF_INET;
					server.sin_port = htons(port);

					if (hp = gethostbyname(address)){
						bcopy(hp->h_addr, (char*)&server.sin_addr, hp->h_length);
					} else if (inet_aton(address, (struct in_addr*) &server.sin_addr)){
						fprintf(stderr, "can't get server address\n");
					}

					if(connect(sd, (struct sockaddr*)&server, sizeof(server)) == -1){
						perror("cant connect\n");
					}

					spdu.type = 'D';
					strcpy(spdu.data, cname);

					write(sd, &spdu, sizeof(struct pdu));


					int contentFile = open(cname, O_RDWR | O_CREAT, 0777);

					read(sd, (struct pdu*)&rpdu, sizeof(struct pdu));
					while(rpdu.type != 'A'){
						write(contentFile, rpdu.data, 100);
						read(sd, (struct pdu*)&rpdu, sizeof(struct pdu));
						
					}


				} else if (rpdu.type == 'E') {
					printf("%s\n", rpdu.data);
				}

				break;
			case 3:
				spdu.type = 'O';
				spdu.data[0] = '\0';
				write(s, &spdu, sizeof(struct pdu));

				read(s, (struct pdu*)&rpdu, sizeof(struct pdu));
				if (rpdu.type != 'O'){
					printf("Error\n");
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

				search = searchName(pname) & searchContent(cname);
				if (!search){
					printf("Content not registered under this user\n");
					break;
				}

				strcpy(spdu.data, pname);
				strcpy(spdu.data+10, cname);
				write(s, &spdu, sizeof(struct pdu));

				read(s, (struct pdu*)&rpdu, sizeof(struct pdu));
				if (rpdu.type == 'A'){
					index = getIndex(search);
					memmove(registeredContent+index, registeredContent+index+1, (--regSize - index) *sizeof(struct registered));
				} else if (rpdu.type == 'E'){
					printf("%s\n", rpdu.data);
				}
				
				break;
			case 5:
				spdu.type = 'T';
				for (i = 0; i < regSize; i++){
					strcpy(spdu.data, registeredContent[i].name);
					strcpy(spdu.data+10, registeredContent[i].contentName);
					write(s, &spdu, sizeof(struct pdu));
				}
				exit(0);
			default:
				printf("Invalid input\n");
		}
	}


	exit(0);
}

uint32_t searchName(char* name){
	int i;
	uint32_t sum = 0;
	for (i = 0; i < regSize; i++){
		if (!strcmp(name, registeredContent[i].name)){
			sum += 1 << i;
		}
	}
	return sum;
}

uint32_t searchContent(char* content){
	int i;
	uint32_t sum = 0;
	for (i = 0; i < regSize; i++){
		if (!strcmp(content, registeredContent[i].contentName)){
			sum += 1 << i;
		}
	}
	return sum;
}

int getIndex(uint32_t bitArray){
	int count = 0;
	while(bitArray != 1){
		bitArray /= 2;
		count++;
	}
	return count;
}
