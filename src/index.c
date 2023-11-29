#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
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
	char address[20];
	char port[6];
};

struct content contents[32];
int contentsSize = 0;
int frequency[32];

//returns bit array of existing content in contents[]
uint32_t searchContent(char* content);
uint32_t searchName(char* name);

//since there can be no duplicates in the contents array
//this assumes there is only a single bit in the array
int getIndex(uint32_t bitArray);

//takes in a bitarray of matching content and returns the index with lowest frequency
int getLowestFrequencyIndex(uint32_t bitArray);

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

	int search, index;
	struct pdu rpdu, spdu;
	char peerName[10], contentName[10];
	while (1) {
		if (recvfrom(s, (struct pdu*)&rpdu, sizeof(struct pdu), 0, (struct sockaddr *)&fsin, &alen) < 0)
			fprintf(stderr, "recvfrom error\n");

		switch (rpdu.type){
			case 'R':
				getpeername(s, (struct sockaddr*) &fsin, &alen);
				struct content cont;

				strcpy(cont.peerName, rpdu.data);
				strcpy(cont.contentName, rpdu.data+10);
				strcpy(cont.address, inet_ntoa(fsin.sin_addr));
				strcpy(cont.port, rpdu.data+20);

				if (searchContent(cont.contentName) & searchName(cont.peerName)) {
					spdu.type = 'E';
					strcpy(spdu.data, "Content already registered with this name");
					(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
					break;
				}

				contents[contentsSize] = cont;
				contentsSize++;

				spdu.type = 'A';
				spdu.data[0] = '\0';
				(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
				break;
			case 'S':
				spdu.type = 'S';
				struct content con;

				strcpy(con.peerName, rpdu.data);
				strcpy(con.contentName, rpdu.data+10);
				
				search = searchContent(con.contentName);
				if(!search){
					spdu.type = 'E';
					strcpy(spdu.data, "Content does not exist");
					(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
					break;
				} else if (searchName(con.peerName) & search) {
					spdu.type = 'E';
					strcpy(spdu.data, "Another peer with the name already registered the content");
					(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
					break;
				}
				index = getLowestFrequencyIndex(search);
				strcpy(spdu.data, contents[index].address);
				strcpy(spdu.data+20, contents[index].port);
				(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));

				frequency[index]++;

				break;

			case 'O':
				spdu.type = 'O';
				sprintf(spdu.data, "%d", contentsSize);
				(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));

				for (i = 0; i < contentsSize; i++){
					sprintf(spdu.data, "name: %s\ncontent name: %s\naddress: %s:%s\n", contents[i].peerName, contents[i].contentName, contents[i].address, contents[i].port);
					(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
				}
				break;
			case 'T':
				strcpy(peerName, rpdu.data);
				strcpy(contentName, rpdu.data+10);

				search = searchName(peerName) & searchContent(contentName);
					
				if (search){
					index = getIndex(search);
					memmove(contents+index, contents+index+1, (--contentsSize - index) *sizeof(struct content));
					frequency[index] = 0;
					spdu.type = 'A';
					spdu.data[0] = '\0';
					(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
				} else {
					spdu.type = 'E';
					strcpy(spdu.data, "Data is not registered\n");
					(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
				}

				break;

			default:
				spdu.type = 'E';
				strcpy(spdu.data, "Type not recognized\n");
				(void) sendto(s, &spdu, sizeof(struct pdu), 0, (struct sockaddr*)&fsin, sizeof(fsin));
		}

	}
}

uint32_t searchName(char* name){
	int i;
	unsigned int sum = 0;
	for (i = 0; i < contentsSize; i++){
		if (!strcmp(name, contents[i].peerName)){
			sum += 1 << i;
		}	
	}
	return sum;
}

uint32_t searchContent(char* content){
	int i;
	unsigned int sum = 0;
	for (i = 0; i < contentsSize; i++){
		if (!strcmp(content, contents[i].contentName)){
			sum += 1 << i;
		}	
	}
	return sum;
}

int getIndex(uint32_t bitArray){
	int count = 0;
	while(bitArray != 1){
		bitArray = bitArray >> 1;
		count++;
	}
	return count;
}

int getLowestFrequencyIndex(uint32_t bitArray){
	int count = 0;
	int lowest = 100;
	int lowestIndex;
	while(bitArray > 0){
		if (bitArray % 2 == 1){
			if(lowest > frequency[count]){
				lowestIndex = count;
				lowest = frequency[count];
			}
		}
		bitArray = bitArray >> 1;
		count++;
	}
	return lowestIndex;
}
