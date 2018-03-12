//compile note: use g++ -std=c++11 -pthread DataNode.cpp -o DataNode
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fstream>
#include <sys/time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <sys/types.h>
#include <thread>
const int MAXPENDING = 99;

using namespace std;

//SERVER SOCKET CODE:
void receiveBlock(int clientSock);
string receiveBlockHelper(int sock, string file_name, long file_size);
string receiveString(int sock);
long receiveLong(int clientSock);
vector<string> blockNames;
void sendHeartbeat(int sock);
void heartbeatThreadTask(char *NameNodeIP, unsigned short NNPort);
int flag; //provides a lock for the threads
void processDataNode(int socket);
void processHeartbeat(string heartbeat_data);

int main(int argc, char const *argv[])
{
  if(argc < 3) {
    cout << "Error: Missing command line arguments" << endl;
    cout << "Usage: ./DataNode [NameNode IP] [portnumber]" << endl;
  return 1;
  }
  flag = 0;
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock < 0) {
    cerr << "Error with socket" << endl;
    exit(-1);
  }
  char* IPAddr = const_cast<char *>(argv[1]);
  unsigned short servPort = atoi(argv[2]);

  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(servPort);
  int status = ::bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if (status < 0) {
    cerr << "Error with MAIN bind" << endl;
    exit (-1);

  }

  status = listen(sock, MAXPENDING);
  if (status < 0) {
    cerr << "Error with listen" << endl;
    exit(-1);
  }

  std::thread threadBeat(heartbeatThreadTask, IPAddr, servPort);

  while(true){
   cout <<"entered while loop!\n"; 

    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientSock = accept(sock, (struct sockaddr *) &clientAddr, &addrLen);
    if (clientSock < 0) {
      cerr << "Error with accept" << endl;
      exit(-1);
    }
    processDataNode(clientSock);
  }
}

void processDataNode(int socket)
{
  string receiveData;  
  while(true){
    receiveData = receiveString(socket);
    if(receiveData == "heartbeat"){
      cout << "Ready to receive heartbeat" << endl;
      string peerIPs  = receiveString(socket);
      processHeartbeat(peerIPs);
      //process all the IPs from the string here and load into peer_IPs vector
    } else if (receiveData == "block"){
      cout << "Ready to receive block" << endl;
      receiveBlock(socket);
    }
  }
  //close(socket);
}

void heartbeatThreadTask(char *NameNodeIP, unsigned short NNPort){
   int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(sock < 0) {
    cout << "THREAD: Error with socket" << endl;
    exit(-1);
   }

  char* IPAddr = const_cast<char *>(NameNodeIP);

  unsigned long servIP;
  int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
  if (status <= 0) {
    cout << "THREAD: Error with convert dotted decimal address to int" << endl;
    exit(-1);
  }

  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = servIP;
  servAddr.sin_port = htons(NNPort);
  status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(status < 0) {
    cout << "THREAD: Error with connect" << endl;
    exit(-1);
  }
  while(true){
    this_thread::sleep_for(chrono::seconds(10));
    cout<< "10-sec heartbeat!\n";
    sendHeartbeat(sock);
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
  while (flag !=0){
    this_thread::sleep_for(chrono::seconds(1));
  } flag = 1;
  blockNames.push_back(file_name);
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
  flag = 0;
  fclose(write_ptr);
  return "success writing\n";
}

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

void sendHeartbeat(int sock){
//sends a single string list of blocks to NameNode
  sendString(sock, "heartbeat");
  string filenames = "";
  while (flag != 0){
    this_thread::sleep_for(chrono::seconds(1));
  } 
  flag = 1;
  for (int i = 0; i<blockNames.size();i++){
    string block = blockNames[i];
    filenames.append(block);
    filenames.append(" ");
  } //now have string for easy sending
  flag = 0; //mark flag open
  sendString(sock, filenames);

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
