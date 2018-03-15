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
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <sys/types.h>
#include <thread>
#include <algorithm>
#include <exception>
const int MAXPENDING = 99;

using namespace std;

//SERVER SOCKET CODE:
void receiveBlock(int clientSock, int replica_flag); 
string receiveBlockHelper(int sock, string file_name, long file_size);
string receiveString(int sock);
long receiveLong(int clientSock);
void replicateBlock(string blockName);
void sendHeartbeat(int sock);
void heartbeatThreadTask(char *NameNodeIP, unsigned short NNPort);
void processDataNode(int socket);
void processHeartbeat(string heartbeat_data);
void sendLong(int clientSock, long size);
void sendString(int sock, string wordSent);
void sendBlockHelper(int sock, string file_name);
void sendBlock(int sock, string file_name);

int flag; //provides a lock for the threads
vector<string> blockNames;
vector<string> peerDataNodeIPs;
int portNo;
int counter = 0; //for determining which replicas to store blocks on

struct NetException : public exception {
  NetException(string reason){
    this->WAT = reason;
  }
   string WAT;
   const char * what () const throw () {
      return WAT.c_str();
   }
};

int main(int argc, char const *argv[])
{
  if(argc < 3) {
    cout << "Error: Missing command line arguments" << endl;
    cout << "Usage: ./DataNode [NameNode IP] [portnumber]" << endl;
  return 1;
  }
  flag = 0;
  portNo = stoi(argv[2]);
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock < 0) {
    cerr << "Error with socket" << endl;
    exit(-1);
  }
  char* IPAddr = const_cast<char *>(argv[1]);
  unsigned short servPort = atoi(argv[2]);
  std::thread threadBeat(heartbeatThreadTask, IPAddr, servPort);

  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(servPort);
  int status = ::bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if (status < 0) {
    cerr << "Error with MAIN bind" << endl;
    exit (-1);
  }

//-------------------------------------------------------TEST CODE
blockNames.push_back("dummy_file");
peerDataNodeIPs.push_back("172.31.25.4");

//-------------------
  status = listen(sock, MAXPENDING);
  if (status < 0) {
    cerr << "Error with listen" << endl;
    exit(-1);
  }

  while(true){
    cout <<"entered while loop!\n"; 
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    cout << "attempting client sock\n";
    int clientSock = accept(sock, (struct sockaddr *) &clientAddr, &addrLen);
    cout << "clientSock received\n";
    if (clientSock < 0) {
      cerr << "Error with accept" << endl;
      exit(-1);
    }
    cout << "launching processDataNode\n";
   //receiveBlock(clientSock, 1);   
    processDataNode(clientSock);
    cout << "Closing socket in function main after processDataNode\n";
    close(clientSock);
  }
}

//Takes list of DataNode peers from NameNode every minute and updates peer node vector
void processHeartbeat(string heartbeat_data){
  cout << "Received peer IP heartbeat!\n Data: " << heartbeat_data << endl;
  string fileName;
  stringstream s (heartbeat_data);
  while(s>> fileName){
    peerDataNodeIPs.push_back(fileName);
    cout << "peer IP: " << fileName << endl;
  }
  //de-dupe the peer vector ID
  sort(peerDataNodeIPs.begin(), peerDataNodeIPs.end());
  peerDataNodeIPs.erase(unique(peerDataNodeIPs.begin(), peerDataNodeIPs.end()), peerDataNodeIPs.end());
}
void processDataNode(int socket)
{
  string receiveData;
  cout << "Entered processDataNode()" << endl;

  //while(true){
    receiveData = receiveString(socket);
    cout << "received message: " << receiveData << endl;
    
    if(receiveData == "heartbeat"){
      cout << "Ready to receive heartbeat" << endl;
      string peerIPs  = receiveString(socket);
      processHeartbeat(peerIPs);
    } else if (receiveData == "block"){
      cout << "Ready to receive NONREPLICA block" << endl;
      receiveBlock(socket, 0);
    } else if (receiveData == "replica"){
      cout << "Ready to receive REPLICA" << endl;
      receiveBlock(socket, 1);
    }
  //}
  //close(socket);
}

void heartbeatThreadTask(char *NameNodeIP, unsigned short NNPort){
   cout << "Heartbeat thread launched!\n";
   
  while(true){
    this_thread::sleep_for(chrono::seconds(10));
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0) {
      cout << "THREAD: Error with socket" << endl;
      exit(-1);
    }

    char* IPAddr = const_cast<char *>(NameNodeIP);
    printf("NameNode IP is: %s\n",NameNodeIP);
    cout << "Port is: " << NNPort << endl;
    unsigned long servIP;
    int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
    if (status <= 0) {
      cout << "THREAD: Error with convert dotted decimal address to int" << endl;
      exit(-1);
    }
    cout << "attempting connection:" << endl;

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET; // always AF_INET
    servAddr.sin_addr.s_addr = servIP;
    servAddr.sin_port = htons(NNPort);
    int connect_attempts = 0;
    status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
    if(status < 0) {
      while(connect_attempts<3){
          status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
          connect_attempts++;
      }
    }
    if (status < 0) {
         cout << "THREAD: Error with connect to NameNode "  << endl;
    } else {
         cout << "10-sec heartbeat!\n" << endl;;
         sendHeartbeat(sock);
         //cout << "Closing socket in function threadTask after Heartbeat sent\n";
         close(sock);
    }
  }
}
//0 = regular block 
//1 = replica block
void receiveBlock(int clientSock, int replica_flag) //based upon processClient
{
  long size;
  string file_name;
  file_name = receiveString(clientSock);
  cout << "File received: " << file_name << endl;
  size = receiveLong(clientSock);
  cout << "Size: " << size << endl;
  string status = receiveBlockHelper(clientSock, file_name, size);
  cout << "Status: " << status << endl;
  if(replica_flag == 0){ //needs replication!
     replicateBlock(file_name);
  cout << "received replica block "<< "file_name!!\n";
  } else{
    cout << "received non-replica block "<< "file_name!!\n";
  }
  cout << "Done with files..." << endl;
  //close(clientSock);
}
void replicateBlock(string blockName){
  try{
    unsigned short servPort = portNo;
    int Node = counter % peerDataNodeIPs.size();
    string IP = peerDataNodeIPs[Node];
    cout << "Attempting peer data node IP: " << IP << "from list of size: " << peerDataNodeIPs.size() << endl; 
    char* IPAddr = const_cast<char *>(IP.c_str());

   int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(sock < 0) {
    cout << "repBlock: Error with socket" << endl;
    exit(-1);
   }
  
  unsigned long servIP;
  int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
  if (status <= 0) {
    cout << "replicateBlock: Error with convert dotted decimal address to int" << endl;
    exit(-1);
  }

  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = servIP;
  servAddr.sin_port = htons(servPort);
  status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(status < 0) {
    cout << "repBlock: Error with connect" << endl;
    exit(-1);
  } //now we have a socket
  sendBlock(sock, blockName);
counter++;
} catch(const std::runtime_error& re) {
          // speciffic handling for runtime_error
          std::cerr << "Replication runtime error: " << re.what() << std::endl;
      }
      catch(const std::exception& ex)
      {
          // speciffic handling for all exceptions extending std::exception, except
          // std::runtime_error which is handled explicitly
          std::cerr << "Error occurred: " << ex.what() << std::endl;
      }
      catch(...)
      {
          // catch any other errors (that we have no information about)
          std::cerr << "Unknown failure occurred" << std::endl;
      }
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
  const unsigned BUF_LEN = 2000;
   unsigned char buffer[BUF_LEN];
   printf("file should be size %ld\n", file_size);
  while(bytesLeft > 0) {
    int bytesRecv = recv(sock, buffer, BUF_LEN, 0);
    if (bytesRecv <= 0) {
      cout << "Error with receiving " << endl;
      exit(-1);
    }
    written = fwrite(buffer,1, bytesRecv,write_ptr);
    bytesLeft = bytesLeft - bytesRecv;//- written;
  cout  << "Remaining: " << bytesLeft << endl;
  }
  flag = 0;
  fclose(write_ptr);
  return "\nsuccess writing\n";
  close(sock);
}  

//C++-based: takes client socket and block file name and
//then sends name and size to sendBlockHelper to send
void sendBlock(int sock, string file_name){

    sendString(sock, file_name);
    //now take file size and send over
    FILE* readPtr;
    readPtr = fopen(file_name.c_str(),"rb");
    fseek(readPtr, 0L, SEEK_END);
    long file_size = ftell(readPtr);
    sendLong(sock, file_size);
    sendBlockHelper(sock, file_name);
    fclose(readPtr);
}
//C-based: sends a binary file to Server by reading out of directory and calculating size
void sendBlockHelper(int sock, string file_name) {
  FILE* readPtr;
  readPtr = fopen(file_name.c_str(),"rb");
  unsigned char binaryBuffer[2000];
  fseek(readPtr, 0L, SEEK_END);
  long file_size = ftell(readPtr);
  rewind(readPtr);

  long remaining_to_send = file_size;

  while(remaining_to_send > 0){
      long b_read = fread(binaryBuffer, 1 , sizeof(binaryBuffer),readPtr);
      int bytesSent = send(sock, (void *) binaryBuffer, b_read, 0);
      if (bytesSent != b_read) {
        cerr << "Error sending " << endl;
        exit(-1);
      }
      remaining_to_send = remaining_to_send - (long)bytesSent;
      cout << "sent " << bytesSent << " remain " << remaining_to_send << "\n";
      fclose(readPtr);
    }
}
void sendLong(int clientSock, long size)
{
  size = htonl(size);
  int bytesSent = send(clientSock, (void *) &size, sizeof(long), 0);
  if(bytesSent != sizeof(long)) {
    pthread_exit(NULL);
  }
}
void sendString(int sock, string wordSent)
{
  char wordBuffer[2000];
  strcpy(wordBuffer, wordSent.c_str());
  int bytesSent = send(sock, (void *) wordBuffer, 2000, 0);
  if (bytesSent != 2000) {
    cerr << "Error sending " << endl;
    cout << "String: " << wordSent << endl;
    printf("Versus: %s\n", wordBuffer);
    exit(-1);
  }
}

void sendHeartbeat(int sock){
//sends a single string list of blocks to NameNode
  sendString(sock, "heartbeat");
  sendString(sock, to_string(portNo));
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
  cout << "Sending heartbeat: " << filenames << endl;
  sendString(sock, filenames);
  //close(sock);
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
