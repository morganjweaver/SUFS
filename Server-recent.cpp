#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cstdlib>
//#include <stdio.h>
#include <cstdio>

using namespace std;

const int MAXPENDING = 99;
string receiveBlock(int sock, string file_name, long file_size);

void sendLong(int clientSock, long size) {
  size = htonl(size);
  int bytesSent = send(clientSock, (void *) &size, sizeof(long), 0);
  if(bytesSent != sizeof(long)) {
    pthread_exit(NULL);
  }
}

void sendString(int clientSock, string stringToSend, int size) {
  sendLong(clientSock, size);
  char msg[size];
  strcpy(msg, stringToSend.c_str());
  if (stringToSend.length() >= size) {
    pthread_exit(NULL);
  }
  int bytesSent = send(clientSock, (void *) msg, size, 0);
  if (bytesSent != size) {
    pthread_exit(NULL);
  }
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


// string receiveString(int clientSock) {
//   long messageSize = receiveLong(clientSock);
//   int bytesLeft = messageSize; // bytes to read
//   char buffer[messageSize];    // initially empty
//   char *bp = buffer;

//   while (bytesLeft) {
//     int bytesRecv = recv(clientSock, bp, bytesLeft, 0);
//     if (bytesRecv <= 0) {
//       exit(-1);
//     }
//     bytesLeft = bytesLeft - bytesRecv;
//     bp = bp + bytesRecv;
//   }
//   string returnString = buffer;
//   return returnString;
// }


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

void processClient(int clientSock)
{
  //long getNum = receiveLong(clientSock);
  //string get = receiveString(clientSock);
  long size;
  string getString;
  while(getString != "exit")
  {
    getString = receiveString(clientSock);
    cout << getString << endl;
    size = receiveLong(clientSock);
    cout << size << endl;
    string status = receiveBlock(clientSock, getString, size);
    cout << status << endl;;
  }
  //If string contains "create filename "
  //
  close(clientSock);
/*
  long messageSize = getNum;
  int bytesLeft = messageSize;
  char buffer[messageSize];
  char *bp = buffer;
  while (bytesLeft) {
    int bytesRecv = recv(clientSock, bp, bytesLeft, 0);
    if (bytesRecv <= 0) {
      pthread_exit(NULL);
    }
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
  string returnString = buffer;
  cout << returnString << endl;
*/

}

string receiveBlock(int sock, string file_name, long file_size) {
 
  FILE *write_ptr;
  write_ptr = fopen(file_name.c_str(),"wb");
  size_t written;
  //long messageSize = receiveLong(sock);
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

int main(int argc, const char * argv[])
{
  if(argc < 2) {
    cout << "Error: check your command line argument" << endl;
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
