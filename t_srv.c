#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>

static long long mstime(void) {
    struct timeval tv;
    long long mst;

    gettimeofday(&tv, NULL);
    mst = ((long long)tv.tv_sec)*1000;
    mst += tv.tv_usec/1000;
    return mst;
}

#define MAXPENDING 5    /* Max connection requests */
#define BUFFSIZE 4096
void Die(char *mess) { perror(mess); exit(1); }
void HandleClient(int sock, int total_trafic) {
	char buffer[BUFFSIZE];
    long total_recv = 0;
	int  msg_count = 0;	
	int received = 0;


	long long start = mstime();
	
	/* Receive message */
	if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
		Die("Failed to receive initial bytes from client");
	}

	total_recv+=received;
	msg_count++;

	
	/* Send bytes and check for more incoming data in loop */
	while (received > 0) {		
		/* Check for more data */
		if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
			printf(" recv error: total_recv:%ld msg_count:%d\n", total_recv, msg_count);
			Die("Failed to receive additional bytes from client");
		}
		
		total_recv+=received;
		msg_count++;

		//printf("total_recv:%ld msg_count:%d\n", total_recv, msg_count);

		if (total_recv >= total_trafic){
			printf("######## total_recv:%ld msg_count:%d\n", total_recv, msg_count);
			long long end = mstime();
		    long long dt = (float)(end-start);
		    float rps = (float)msg_count*1000/dt;
			printf("######## start_time:%lld end_time:%lld dt:%lld\n", start,end,dt);
			printf("######## rps:%.2f\n", rps);
			printf("######## rate:%.2f Mbps\n", (float)total_recv*8*1000/(dt*1024*1024));			
		}
	}

	printf("HandleClient exits\n");
	
	close(sock);
}

int main(int argc, char *argv[]) {
	int serversock, clientsock;
	struct sockaddr_in echoserver, echoclient;

	if (argc != 3) {
		fprintf(stderr, "USAGE: echoserver <port> <total_traffic(M)>\n");
		exit(1);
	}
	int total_trafic = 1024*1024*100;
	total_trafic = atoi(argv[2])*1024*1024;
	printf("max total_trafic:%d\n", total_trafic);
	
	/* Create the TCP socket */
	if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		Die("Failed to create socket");
	}
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
	echoserver.sin_port = htons(atoi(argv[1]));       /* server port */

	/* Bind the server socket */
	if (bind(serversock, (struct sockaddr *) &echoserver,
				sizeof(echoserver)) < 0) {
		Die("Failed to bind the server socket");
	}
	/* Listen on the server socket */
	if (listen(serversock, MAXPENDING) < 0) {
		Die("Failed to listen on server socket");
	}
	/* Run until cancelled */
	while (1) {
		unsigned int clientlen = sizeof(echoclient);
		/* Wait for client connection */
		if ((clientsock =
					accept(serversock, (struct sockaddr *) &echoclient,
						&clientlen)) < 0) {
			Die("Failed to accept client connection");
		}
		fprintf(stdout, "Client connected: %s\n",
				inet_ntoa(echoclient.sin_addr));
		HandleClient(clientsock, total_trafic);
	}
}
