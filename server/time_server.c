/* time_server.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

struct pdu {
	char type;
	char data[100];
};

/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	char	buf[100];		/* "input" buffer; any size > 0	*/
	char    *pts;
	int	sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
        int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;
	int	i, total;
                                                                                

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

	while (1) {
		struct pdu rpdu;
		if (recvfrom(s, (struct pdu*)&rpdu, sizeof(struct pdu), 0,
				(struct sockaddr *)&fsin, &alen) < 0)
			fprintf(stderr, "recvfrom error\n");
		int fd;
		struct pdu spdu;
		if (rpdu.type == 'C'){
			if (fd = open(rpdu.data, O_RDONLY) == -1){
				spdu.type = 'E';
				strcpy(spdu.data, "File not found");
				(void) sendto(s, &spdu, sizeof(struct pdu), 0,
						(struct sockaddr *)&fsin, sizeof(fsin));
			} else {
				spdu.type = 'D';
				FILE* fp = fopen(rpdu.data, "r");
				if (fp == NULL) { 
					printf("File Not Found!\n"); 
					return -1; 
				}
				fseek(fp, 0L, SEEK_END);
				long int length = ftell(fp);
				rewind(fp);
				fclose(fp);

				fd = open(rpdu.data, O_RDONLY);
				total = 0;
				while (length - total > 100){
					i = read(fd, buf, 100);
					strcpy(spdu.data, buf);
					(void) sendto(s, &spdu, sizeof(struct pdu), 0,
							(struct sockaddr *)&fsin, sizeof(fsin));
					total += i;
				}
				spdu.type = 'F';
				i = read(fd, buf, 100);
				buf[length - total + 1] = '\0';

				strcpy(spdu.data, buf);
				(void) sendto(s, &spdu, sizeof(struct pdu), 0,
						(struct sockaddr *)&fsin, sizeof(fsin));
			}
		}
	}
}
