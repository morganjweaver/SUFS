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
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>

//#include "NodeHashMap.cpp"
//#include "DirHashMap.cpp"

const int SUCCESS = 0;
const int FILE_NOT_EXIST = 1;
const int PATH_NOT_EXIST = 2;
const int FILE_EXISTS = 3;
const int DIRECTORY_EXIST = 4;
const int DIRECTORY_NOT_EMPTY = 5;

#define PORT 8080
const int MAXPENDING = 99;

using namespace std;

//const char* delimiter = '~';
void processClient(int new_client_socket);
void sendString(int sock, string wordSent);
string receiveString(int sock);
void processClient(int clientSock);

//SERVER SOCKET CODE
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
    processClient(clientSock);
  }
}
/******************************************************************************/

void sendString(int sock, string wordSent)
{
  char wordBuffer[2000];
  strcpy(wordBuffer, wordSent.c_str());
  int bytesSent = send(sock, (void *) wordBuffer, 2000, 0);
  if (bytesSent != 2000) {
    cerr << "Error sending " << endl;
    exit(-1);
  }
}

string receiveString(int sock)
{
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

void processClient(int clientSock)
{
  string command;
  string getName;
  string getPath;

  while(true)
  {
    command = receiveString(clientSock);
    cout << command << endl;
    //if command is a write block to file, add recieveBlock
    if(command == "exit"){
      close(clientSock);
      exit(-1);
    }

    if(command == "mkdir")
    {
      getName = receiveString(clientSock);
      cout << getName << endl;
      getPath = receiveString(clientSock);
      cout << getPath << endl;
    } else if(command == "ls") {
      getPath = receiveString(clientSock);
      cout << getPath << endl;
    } else if (command == "create") { //integrate receiveBlock here
      getName = receiveString(clientSock);
      cout << getName << endl;
      getPath = receiveString(clientSock);
      cout << getPath << endl;
    } else if (command == "stat") {
      getPath = receiveString(clientSock);
      cout << getPath << endl;
    } else if (command == "rmdir"){
      getPath = receiveString(clientSock);
      cout << getPath << endl;
    } else if (command == "cat"){
      getPath = receiveString(clientSock);
      cout << getPath << endl;
    }
  } //end while
}


