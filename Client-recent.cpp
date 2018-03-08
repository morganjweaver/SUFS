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

using namespace std;

void sendLong(int clientSock, long size) {
  size = htonl(size);
  int bytesSent = send(clientSock, (void *) &size, sizeof(long), 0);
  if(bytesSent != sizeof(long)) {
    pthread_exit(NULL);
  }
}

/*
void sendString(int clientSock, string stringToSend, int size) {
  sendLong(clientSock, size);
  char msg[size];
  strcpy(msg, stringToSend.c_str());

  if (stringToSend.length() > size) {
    exit(-1);
  }

  int bytesSent = send(clientSock, (void *) msg, size, 0);
  if (bytesSent != size) {
    exit(-1);
  }
}
*/

void sendString(int sock, string wordSent) {
  char wordBuffer[2000];
  strcpy(wordBuffer, wordSent.c_str());
  int bytesSent = send(sock, (void *) wordBuffer, 2000, 0);
  if (bytesSent != 2000) {
    cerr << "Error sending " << endl;
    exit(-1);
  }
}

long receiveLong(int clientSock) {
  int bytesLeft = sizeof(long);  // bytes to read
  long numberGiven;   // initially empty
  char *bp = (char *) &numberGiven;
  while (bytesLeft) {
    int bytesRecv = recv(clientSock, bp, bytesLeft, 0);
    if (bytesRecv <= 0) {
      pthread_exit(NULL);
    }
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
  long hostToInt = ntohl(numberGiven);
  return hostToInt;
}

string receiveString(int clientSock) {
  long messageSize = receiveLong(clientSock);
  int bytesLeft = messageSize; // bytes to read
  char buffer[messageSize];    // initially empty
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
  return returnString;
}

int main(int argc, char const *argv[]) {
  if(argc < 3) {
    cout << "Error: check your command line argument" << endl;
    cout << "Usage: ./Cient [ip_address] [portnumber]" << endl;
    return 1;
  }

  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock < 0) {
    cout << "Error with socket" << endl;
    exit(-1);
  }

  char* IPAddr = const_cast<char *>(argv[1]);

  unsigned short servPort = atoi(argv[2]);

  unsigned long servIP;
  int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
  if (status <= 0) {
    cout << "Error with convert dotted decimal address to int" << endl;
    exit(-1);
  }

  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = servIP;
  servAddr.sin_port = htons(servPort);

  status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(status < 0) {
    cout << "Error with connect" << endl;
    exit(-1);
  }

  //long sendNum = 10;
  //sendLong(sock, sendNum);
  string input;



  while(input != "exit"){
    cout << ">> ";
    cin >> input;
    sendString(sock, input);
  }

  close(sock);
  return 0;
}
