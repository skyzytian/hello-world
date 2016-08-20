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

#define BUFFSIZE 32
void Die(char *mess) { perror(mess); exit(1); }
int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in echoserver;
	unsigned int echolen;
	int received = 0;
	int payload_len = 128;

#ifdef __linux__
#endif 
#ifdef USE_SELECT
#endif 
#ifdef __USE_GNU
//printf("__USE_GNU\n");
#endif
	if (argc != 5) {
		fprintf(stderr, "USAGE: TCPecho <server_ip> <port> <total_traffic(M)> <payload>\n");
		exit(1);
	}

	printf("starting the client\n");

	int total_trafic = 1024*1024*100;
	total_trafic = atoi(argv[3])*1024*1024;
	payload_len = atoi(argv[4]);
	printf("max total_trafic:%d payload_len:%d\n", total_trafic, payload_len);
	
    struct sockaddr_in clientaddr;
	memset(&clientaddr, 0, sizeof(clientaddr));
    //clientaddr.sin_addr.s_addr = inet_addr("10.56.249.171");  /* IP address */
    //clientaddr.sin_addr.s_addr = inet_addr("192.168.23.217");  /* IP address */
    clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientaddr.sin_port = htons(0); 
    /* Create the TCP socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            printf("Failed to create socket\n");
    }  

    if (bind(sock, (struct sockaddr *) &clientaddr, sizeof(clientaddr)) < 0) {
            printf("Failed to bind the socket\n");
            return -1; 
    } 

	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(argv[1]);  /* IP address */
	echoserver.sin_port = htons(atoi(argv[2]));       /* server port */
	/* Establish connection */
	if (connect(sock,(struct sockaddr *) &echoserver,
				sizeof(echoserver)) < 0) {
		Die("Failed to connect with server");
	}
	
	long long start = mstime();
	int len = payload_len;
	long total_send = 0;
	int  msg_count = 0;	
	int sendmsg = 0;
	char *data = malloc(len);
	memset(data,'x',len-1);
	data[len-1]='\0';
	int size = strlen(data);
	while (1) {
		sendmsg = send(sock, data, size, 0);
		if ( sendmsg < 0) 
		{
			printf("send error: total_send:%ld msg_count:%d\n", total_send, msg_count);
			Die("send message error");
		}
		
		total_send += sendmsg;	
		msg_count++;

		if (total_send > total_trafic){
			printf("######## total_send:%ld msg_count:%d\n", total_send, msg_count);
			break;
		}
	}
	long long end = mstime();
    long long dt = (float)(end-start);
    float rps = (float)msg_count*1000/dt;
	printf("######## start_time:%lld end_time:%lld dt:%lld\n", start,end,dt);
	printf("######## rps:%.2f\n", rps);
	printf("######## rate:%.2f Mbps\n", (float)total_send*8*1000/(dt*1024*1024));

	sleep(5);

	fprintf(stdout, "\n");
	close(sock);
	exit(0);
}
