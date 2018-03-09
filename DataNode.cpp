#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <stdio.h>
#include <fstream>
using namespace std;

//TODO: 
// Create listener for incoming requests
// Create handler for incoming requests from listener

// Create update function to send dir info to NameNode
// Create DeleteBlock
// Create StoreBlock
// 
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080
#define MAXBUF 32
//SERVER SOCKET CODE:
void receiveBlock(int clientSock);
string receiveBlockHelper(int sock, string file_name, long file_size);
string receiveString(int sock);
long receiveLong(int clientSock);

// int main(int argc, char const *argv[])
// {
//     int server_fd, new_socket, valread;
//     struct sockaddr_in address;
//     int opt = 1;
//     int addrlen = sizeof(address);
//     char buffer[MAXBUF] = {0};
//     char *hello = "Hello from server";
      
//     // Creating socket file descriptor
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
//     {
//         perror("socket failed");
//         exit(EXIT_FAILURE);
//     }
      
//     // Forcefully attaching socket to the port 8080
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
//                                                   &opt, sizeof(opt)))
//     {
//         perror("setsockopt");
//         exit(EXIT_FAILURE);
//     }
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons( PORT );
      
//     // Forcefully attaching socket to the port 8080
//     if (bind(server_fd, (struct sockaddr *)&address, 
//                                  sizeof(address))<0)
//     {
//         perror("bind failed");
//         exit(EXIT_FAILURE);
//     }
//     if (listen(server_fd, 3) < 0)
//     {
//         perror("listen");
//         exit(EXIT_FAILURE);
//     }
//     if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
//                        (socklen_t*)&addrlen))<0)
//     {
//         perror("accept");
//         exit(EXIT_FAILURE);
//     }
//     ofstream outfile ("testOUTFILE.txt");
//     while((valread = recv(new_socket, buffer, MAXBUF, 0)) > 0){
//         if(valread <= 0)
//             break;
//     else {
//         buffer[valread] = '\0'; 
//         outfile << buffer; 
//         printf("%s", buffer);
//     }
// }
//     outfile.close();
//    // valread = read( new_socket , buffer, 1024);
//    // printf("%s\n",buffer );
//     send(new_socket , hello , strlen(hello) , 0 );
//     printf("Hello message sent\n");
//     return 0;
// }
int main(int argc, char const *argv[])
{
  if(argc < 2) {
    cout << "Error: Missing command line arguments" << endl;
    cout << "Usage: ./Server [portnumber]" << endl;
  return 1;
  }

  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock < 0) {
    cerr << "Error with socket" << endl;
    exit(-1);
  }

   unsigned short servPort = atoi(argv[1]);

  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(servPort);

  int status = bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if (status < 0) {
    cerr << "Error with bind" << endl;
    exit (-1);
  }

  status = listen(sock, MAXPENDING);
  if (status < 0) {
    cerr << "Error with listen" << endl;
    exit(-1);
  }

  while(true){
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientSock = accept(sock, (struct sockaddr *) &clientAddr, &addrLen);
    if (clientSock < 0) {
      cerr << "Error with accept" << endl;
      exit(-1);
    }
    receiveBlock(clientSock);
  }
}

void receiveBlock(int clientSock) //based upon processClient
{

  long size;
  string file_name;
  while(file_name != "exit")
  {
    file_name = receiveString(clientSock);
    cout << "File received: " << file_name << endl;
    size = receiveLong(clientSock);
    cout << "Size: " << size << endl;
    string status = receiveBlockHelper(clientSock, file_name, size);
    cout << "Status: " << status << endl;
  }
 
  //close(clientSock);

}

string receiveBlockHelper(int sock, string file_name, long file_size) {
 
  FILE *write_ptr;
  write_ptr = fopen(file_name.c_str(),"wb");
  size_t written;
  int bytesLeft = file_size;
  const unsigned BUF_LEN = 2048;
   unsigned char buffer[BUF_LEN];
   printf("file should be size %ld", file_size);
  while(bytesLeft > 0) {
    int bytesRecv = recv(sock, buffer, BUF_LEN, 0);
    cout << "got " << bytesRecv << " from client\n";
    if (bytesRecv <= 0) {
      cout << "Error with receiving " << endl;
      exit(-1);
    }
    written = fwrite(buffer,1, bytesRecv,write_ptr);
    bytesLeft = bytesLeft - bytesRecv;//- written;
    printf("wrote %i to file\n", (int)written);
  }
  fclose(write_ptr);
  return "success writing\n";
}

string receiveString(int sock) {
  int bytesLeft = 2000;
  char stringBuffer[2000];
  while(bytesLeft) {
    int bytesRecv = recv(sock, stringBuffer, bytesLeft, 0);
    if (bytesRecv < 0) {
      cout << "Error with receiving " << endl;
      exit(-1);
    }
    bytesLeft = bytesLeft - bytesRecv;
  }
  return stringBuffer;
}
long receiveLong(int clientSock) {
  int bytesLeft = sizeof(long);  // bytes to read
  long numberGiven;   // initially empty
  char *bp = (char *) &numberGiven;
  while (bytesLeft) {
    int bytesRecv = recv(clientSock, bp, bytesLeft, 0);
    if (bytesRecv <= 0) {
      exit(-1);
    }
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
  long hostToInt = ntohl(numberGiven);
  return hostToInt;
}
