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
string receiveBlockHelper(int sock, string file_name, long file_size, int replica_flag);
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
int getSocket(char*IP, unsigned short port);
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
  //Load blocks if available
  int flength;
  ifstream filestr;

  filestr.open("blockbackup.txt", ios::binary); // open your file
  filestr.seekg(0, ios::end); // put the "cursor" at the end of the file
  flength = filestr.tellg(); // find the position of the cursor
  filestr.close(); // close your file

if ( flength != 0 )
  {
    //read in the file names line at a time:
    ifstream infile("blockbackup.txt");
    string filename;
    while(infile >> filename){
      blockNames.push_back(filename);
      cout << "Just recovered filename " << filename << endl;
    }
    infile.close();
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
    processDataNode(clientSock);
  //  cout << "Closing socket in function main after processDataNode\n";
    //close(clientSock);
  }
}

//Takes list of DataNode peers from NameNode every minute and updates peer node vector
void processHeartbeat(string heartbeat_data){
  cout << "Received peer IP heartbeat!\n Data: " << heartbeat_data << " " << endl;
  string fileName;
  stringstream s (heartbeat_data);
  while(s>> fileName){
    peerDataNodeIPs.push_back(fileName);
    cout << "peer IP: " << fileName << " " << endl;
  }
  //de-dupe the peer vector ID
  sort(peerDataNodeIPs.begin(), peerDataNodeIPs.end());
  peerDataNodeIPs.erase(unique(peerDataNodeIPs.begin(), peerDataNodeIPs.end()), peerDataNodeIPs.end());
}
void processDataNode(int socket)
{
  string receiveData;

    receiveData = receiveString(socket);
    cout << "received message: " << receiveData << endl;
    
    if(receiveData == "heartbeat"){
      cout << "Ready to receive heartbeat" << endl;
      string peerIPs  = receiveString(socket);
      processHeartbeat(peerIPs);
    } else if (receiveData == "block"){
      cout << "About to receive block" << endl;
      receiveBlock(socket, 0);
    } else if (receiveData == "replicate"){
      string filename = receiveString(socket);//file name to replicate
      cout << "Received command to replicate block " << filename << endl;
      sendBlock(socket, filename);
    } else{
      cout << "MAIN PROCESS ERROR: Command matches no known functionality!\n";
    }
    cout << "Closing socket in processDataNode\n";
    close(socket);
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
    //printf("NameNode IP is: %s\n",NameNodeIP);
    cout << "Port is: " << NNPort << endl;
    unsigned long servIP;
    int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
    if (status <= 0) {
      cout << "THREAD: Error with convert dotted decimal address to int" << endl;
      exit(-1);
    }
    //cout << "attempting connection:" << endl;

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

void replicateBlock(string blockName){
  try{
    cout << "ABOUT TO TRY REPLICATE!!!\n\n";
   
    int Node = counter % peerDataNodeIPs.size();
    string IP = peerDataNodeIPs[Node];
    int n = IP.length();
    char DN_IP[n+1];
    strcpy(DN_IP, IP.c_str());
    //char* IPAddr = const_cast<char *>(DN_IP);
    
    cout << "Attempting peer data node IP: " << IP << " from list of size: " << peerDataNodeIPs.size() << endl; 
  unsigned short port = (unsigned short)portNo;
  int sock = getSocket(DN_IP, port);  
  cout << "now sending REPLICATE command to peer\n";
  sendString(sock, "block");
  sendBlock(sock, blockName);
//counter++;
//close(sock);
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
  string status = receiveBlockHelper(clientSock, file_name, size, replica_flag);
  cout << "Status: " << status << endl;
  cout << "Done with files..." << endl;
}
string receiveBlockHelper(int sock, string file_name, long file_size, int replica_flag) {
 
  cout << "ReceiveHelper: Getting block " << file_name << endl;
  blockNames.push_back(file_name);
  flag = 0;
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
  
  }
  fclose(write_ptr);
  cout  << "Received block "<<file_name << endl;
  cout << "*************************\n";
  cout << "Blocks held: \n";
  for (int i = 0; i<blockNames.size(); i++)
    cout << blockNames[i] << "\n";
  cout << "*************************\n";
  string blockname = file_name;
  blockname.append("\n");
  ofstream out("blockbackup.txt", ios::app);
  out << blockname;
  out.close();
  return "\nsuccess writing\n";
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
  cout << "sendblockhelper sending " << file_name << endl;
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
      //cout << "sent " << bytesSent << " remain " << remaining_to_send << "\n";
     
    }
    cout << "SENT COMPLETE for block " << file_name << endl;
     fclose(readPtr);
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
    replace(block.begin(), block.end(), '_', '/');
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

int getSocket(char*IP, unsigned short port){
    cout << "SOCKET GOT\n\n";

  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock < 0) {
    cout << "Error with socket" << endl;
    exit(-1);
  }

  char* IPAddr = const_cast<char *>(IP);
 

  unsigned long servIP;
  int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
  if (status <= 0) {
    cout << "Error with convert dotted decimal address to int" << endl;
    exit(-1);
  }

  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = servIP;
  servAddr.sin_port = htons(port);
  status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(status < 0) {
    cout << "Error with connect" << endl;
    exit(-1);
  }
  //sendString(sock, "SANITY_CHECK");
  return sock;
}
