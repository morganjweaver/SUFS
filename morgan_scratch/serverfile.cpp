#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUF 1024
#define S_PORT  8080

int main()
{
	int sockd,sockd2;
	int addrlen;
	struct sockaddr_in my_name, peer_name;
	int status;

	/* create a socket */
	sockd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockd == -1)
	{
		perror("Tworzenie gniazda");
		return 1;
	}

	/* socket binding */
	my_name.sin_family = AF_INET;
	my_name.sin_addr.s_addr = INADDR_ANY;
	my_name.sin_port = htons(S_PORT);

	status = bind(sockd, (struct sockaddr*)&my_name, sizeof(my_name));
	if (status == -1)
	{
		perror("Binding");
		return 1;
	}

	status = listen(sockd, 5);
	if (status == -1){
		perror("Listening");
		return 1;
	}

	for(;;)
	{
		int fd;
		int i, count_r, count_w;
		char* bufptr;
		char buf[MAX_BUF];
		char filename[MAX_BUF];

		/* wait for an incoming connection */
	//	addrlen = sizeof(peer_name);
		unsigned addrLen = (unsigned) sizeof(peer_name);
		sockd2 = accept(sockd, (struct sockaddr*)&peer_name, (socklen_t *)&addrlen);
		if (sockd2 == -1)
		{
			perror("Connection accept");
			return 1;
		}

		i = 0;
		if ((count_r = read(sockd2, filename + i, MAX_BUF))>0)
		{
			i += count_r;
		}
		filename[i-1] = '\0';
		if (count_r == -1){
			perror("Read error");
			return 1;
		}
		printf("Trying to read file %s\n", filename);

		fd = open(filename, O_RDONLY); 
		if (fd == -1)
		{
			perror("File open error");
			return 1;
		}
		while((count_r = read(fd, buf, MAX_BUF))>0)
		{
			count_w = 0;
			bufptr = buf;
			while (count_w < count_r)
			{
				count_r -= count_w;
				bufptr += count_w;
				count_w = write(sockd2, bufptr, count_r);
				if (count_w == -1) 
				{
					perror("Socket read error");
					return 1;
				}
			}
		}
		close(fd);
		close(sockd2);
	}
	return 0;
}