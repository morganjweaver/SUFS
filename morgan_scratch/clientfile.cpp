#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUF 1024
#define S_PORT  8080

int main(int argc, char* argv[])
{
	int sockd;
	int count;
	struct sockaddr_in serv_name;
	char buf[MAX_BUF];
	int status;

	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s ip_address filename\n", argv[0]);
		return 1;	}
	/* create a socket */
	sockd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockd == -1)
	{
		perror("Socket creation");
		return 1;
	}

	/* server address */ 
	serv_name.sin_family = AF_INET;
	inet_aton(argv[1], &serv_name.sin_addr);
	serv_name.sin_port = htons(S_PORT);

	/* connect to the server */
	status = connect(sockd, (struct sockaddr*)&serv_name, sizeof(serv_name));
	if (status == -1)
	{
		perror("Connection error");
		return 1;
	}

	write(sockd, argv[2], strlen(argv[2])+1);
	shutdown(sockd, 1);
	while ((count = read(sockd, buf, MAX_BUF))>0)
	{
		write(1, buf, count);
	}
	if (count == -1)
	{
		perror("Read error");
		return 1;
	}

	close(sockd);
	return 0;
}