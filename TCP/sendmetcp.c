#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h> 

//#define PORT "4950" // the port client will be connecting to 
#define MAXDATASIZE 1400 // max number of bytes we can get at once 

/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */

int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y){
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
	int sockfd;
	struct addrinfo hints, *serverinfo, *p;
	int rv;
	int numbytes,numread;
	char buffer[MAXDATASIZE];
	char s[INET6_ADDRSTRLEN];
	char trunk[]="[Segemented]";

	// const char* separator = strrchr(argv[0], ':');
	// if ( separator ){
	// 	PORT = separator + 1;
	// } else {
	// 	PORT = argv[0];
	// }

	struct timeval GTOD_before, GTOD_after, difference; 
	
	char** tokens;
	FILE *fptr;
	
	if (argc != 4) {
		fprintf(stderr,"usage: %s <HOSTNAME> <PORT> <FILENAME>\n",argv[0]);
		exit(1);
	}

	// 结构体初始化
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	// 准备文件属性
	//char delim[] = ":";
	// char *Desthost=strtok(argv[1],delim);
	// char *Destport=strtok(NULL,delim);
	char *Desthost=argv[1];
	char *Destport=argv[2];
	char *filename=argv[3];
	int totalBytes = 0;
	// 打开文件
	fptr = fopen(filename,"r");
	if(fptr == NULL){
		// 文件不存在
        perror("File Not Found\n");
        exit(1);
    } else {
		printf("File %s is sending to %s:%s \n",filename,Desthost,Destport);
	
		// 判断连接
		if ((rv = getaddrinfo(argv[1], Destport, &hints, &serverinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		// loop through all the results and connect to the first we can
		for(p = serverinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				perror("client: socket error\n");
				continue;
			}

			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(sockfd);
				perror("client: connect error\n");
				continue;
			}

			break;
		}
		
		if (p == NULL) {
			fprintf(stderr, "client: failed to connect\n");
			return 2;
		}
		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
		printf("Client connecting to %s\n", s);

		freeaddrinfo(serverinfo);

		// 发送文件
        gettimeofday(&GTOD_before,NULL);

        bzero(buffer, MAXDATASIZE);

		//int totalBytes = 0;
		int tempBytes = 0;

		while((tempBytes = (int)fread(buffer, sizeof(char), MAXDATASIZE, fptr)) > 0){
			// 判断读取是否错误
			if (tempBytes == 0){
				perror("File reading error.\n");
				exit(1);
			}
			// 发送文件
			numbytes = send(sockfd, buffer, MAXDATASIZE-1, 0);
			if(numbytes == -1 ){
				perror("Sending error.\n");
				exit(1);
			}
			if (numbytes == 0) {
				printf("Server closed.\n");
				break;
			}
			totalBytes += numbytes;
			
			// printf("Client (%d/%d) : sended: %s\n",numbytes, totalBytes, buffer);
			bzero(buffer,MAXDATASIZE);
		}
	}
	fclose(fptr);
	close(sockfd);
	
	gettimeofday(&GTOD_after,NULL); 
	
	timeval_subtract(&difference,&GTOD_after,&GTOD_before);

	if (difference.tv_sec > 0) {
		printf("Total bytes %d sended and Time used: %ld s %06ld ms [us].\n", totalBytes, difference.tv_sec, difference.tv_usec);
	} else {
		printf("Total bytes %d sended and Time used: %6ld ms [us].\n", totalBytes, difference.tv_usec);
	}

	return 0;
}