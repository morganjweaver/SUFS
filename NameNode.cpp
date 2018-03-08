//TODO:
// Create listener for incoming requests
// Create handler for incoming requests from listener
// Create Put/WriteBlock
// Create car/ls function
// Create Stat
// Create DeleteBlock
// Create StoreBlock
// Create Stat
// Create Hashtable data structure for file dir
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
//#include "NodeHashMap.cpp"
#include "DirHashMap.cpp"

#define PORT 8080

using namespace std;

const char* delimiter = '~';
void processClient(int new_client_socket);

//SERVER SOCKET CODE
int main(int argc, char const *argv[])
{
    int server_fd, client_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    DirHashMap dirMap;
    NodeHashMap nodeMap;
  

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (true) 
        {   struct sockaddr_in clientAddr;
            socklen_t addrLen = sizeof(clientAddr);
            clientSock = accept(server_fd,(struct sockaddr *)&clientAddr, &addrLen);
            if (clientSock < 0) 
                exit(-1);
    // Communicate with client
            processClient(client_socket);
    // Close client 
            close(clientSock);
        }
    // if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
    //                    (socklen_t*)&addrlen))<0)
    // {
    //     perror("accept");
    //     exit(EXIT_FAILURE);
    // }

    // valread = read( new_socket , buffer, 1024);
    // printf("%s\n",buffer );
    // char *result = clientRequestHandler(buffer);
    
    // send(new_socket, delimiter, strlen(delimiter), 0);
    // send(new_socket , result , strlen(result) , 0 );  //send back useful info to client based on request
    // send(new_socket, delimiter, strlen(delimiter), 0); //now cap it with a delimiter


    // //send(new_socket , result , strlen(result) , 0 );

    // printf("Return message sent\n");
    return 0;
}

void processClient(int new_client_socket){

    char buffer[1024] = {0};
    char* message = "recieved message!";
    valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer );
    
    send(new_socket, delimiter, strlen(delimiter), 0);
    send(new_socket , message , strlen(result) , 0 );  //send back useful info to client based on request
    send(new_socket, delimiter, strlen(delimiter), 0); //now cap it with a delimiter

}

//CLIENT SOCKET CODE
/*int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );
    return 0;
}
*/
