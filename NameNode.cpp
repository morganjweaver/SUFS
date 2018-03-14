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
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <thread>
#include "DirHashMap.cpp"
#include "NodeHashMap.cpp"
#define PORT 8080

using namespace std;

//Error messages to send back to client

const int MAXPENDING = 99;

void processClient(int new_client_socket);
void sendString(int sock, string wordSent);
string receiveString(int sock);
void processClient(int clientSock, string clientIP);
void processHeartbeat(string clientPort, string nodeIPaddr, string heartbeat_data);
int uniqueIDCounter = 0;
bool mkdir(string name, string path, DirHashMap& dirMap);
bool rmdir(string name, string path, DirHashMap& dirMap);
vector<string> ls(string path, DirHashMap& dirMap);
vector<string> DataNodeIPs;
void heartbeatThreadTask();
void sendHeartbeat(int sock, string IPstring);
bool create(string name, string path, vector<string> chunkID, vector<string> dataNodeIP, DirHashMap& dirMap);
long receiveLong(int clientSock);

string DataNodePort = "0";
//SERVER SOCKET CODE
int main(int argc, char const *argv[])
{
  if(argc < 2) {
    cout << "Error: Missing command line arguments" << endl;
    cout << "Usage: ./NameNode [portnumber]" << endl;
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

  int status = ::bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if (status < 0) {
    cerr << "Error with bind" << endl;
    exit (-1);
  }

  status = listen(sock, MAXPENDING);
  if (status < 0) {
    cerr << "Error with listen" << endl;
    exit(-1);
  }

  std::thread threadBeat(heartbeatThreadTask);
  DataNodeIPs.push_back("172.31.19.92");
  while(true){
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientSock = accept(sock, (struct sockaddr *) &clientAddr, &addrLen);
    if (clientSock < 0) {
      cerr << "Error with accept" << endl;
      exit(-1);
    }

    //Grabs client IP address for DataNode ID
    socklen_t len;
    struct sockaddr_storage addr;
    char ipstr[INET6_ADDRSTRLEN];
      int port;

    len = sizeof addr;
    getpeername(clientSock, (struct sockaddr*)&addr, &len);

    // deal with both IPv4 and IPv6:
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *clientSock = (struct sockaddr_in *)&addr;
        port = ntohs(clientSock->sin_port);
        inet_ntop(AF_INET, &clientSock->sin_addr, ipstr, sizeof ipstr);
    } else { // AF_INET6
        struct sockaddr_in6 *clientSock = (struct sockaddr_in6 *)&addr;
        port = ntohs(clientSock->sin6_port);
        inet_ntop(AF_INET6, &clientSock->sin6_addr, ipstr, sizeof ipstr);
  }

    processClient(clientSock, ipstr);
    cout << "Closing socket in function main in while loop\n";
    close(clientSock);
  }
}

/******************************************************************************/
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
//add heartbeat information to DataNode:block hashtable

void processHeartbeat(string clientPort, string nodeIPaddr, string heartbeat_data, NodeHashMap& nodeMap){
    //if put attempt returns false, remove the entry and try again
  cout << "Heartbeat received from " << nodeIPaddr << " contents: " << heartbeat_data << endl;
  if(DataNodePort != clientPort){
    cout << "DatNodePort set to " << clientPort << " from " <<DataNodePort << endl;
   DataNodePort = clientPort;
  }
  DataNodeIPs.push_back(nodeIPaddr);
  //now de-dupe from global IP list
  for(int i = 0; i<DataNodeIPs.size()-1; i++){
    if (DataNodeIPs[i] == nodeIPaddr)
        DataNodeIPs.erase(DataNodeIPs.begin()+i);
  }
  vector<string> blockFileNames;
  string fileName;
  stringstream s (heartbeat_data);
  while(s>> fileName)
    blockFileNames.push_back(fileName);
  // Now we have a vector of block file IDs and the IP addr of the DataNode that holds them
  // Add global hashmap fof block-->vector<string file> table here!!

   if(nodeMap.put(nodeIPaddr, blockFileNames) == false){
     cout << "failed to put addresses" << endl;
     exit(-1);
   }
   

}
/*
* This function processes the commands received from the client.
Will work with the hash tables to update the directory/file lookup table, and ID lookup table
*/
void processClient(int clientSock, string clientIP)
{
  string command;
  string getName;
  string getPath;
  DirHashMap dirMap;
  NodeHashMap nodeMap;
  vector<string> lsReturn;
  bool check = false;
  //while(true)
  //{
    command = receiveString(clientSock);
    //cout << command << endl;

    //if command is a write block to file, add recieveBlock

    //if client exits, close the server
    if(command == "exit"){
      close(clientSock);
      exit(-1);
    }

    if(command == "heartbeat"){
      string port = receiveString(clientSock); 
      string blocks = receiveString(clientSock);

      processHeartbeat(port, clientIP, blocks,  nodeMap);
      //close(clientSock);
    }

    if(command == "mkdir")
    {
      getName = receiveString(clientSock);
      cout << getName << endl;
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      check = mkdir(getName, getPath, dirMap);
			sendLong(clientSock, check);
			if(check == 1)
				cout << "Success" << endl;
			else
				cout << "Error" << endl;
      //cout << check << endl;

			cout << endl;
    }
    else if(command == "ls")
    {
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      lsReturn = ls(getPath, dirMap);

			if(lsReturn.size() == 0){
				cout << "Error" << endl;
			}

			for(int i = 0; i < lsReturn.size(); i++){
				cout << lsReturn[i] << endl;
			}

      sendLong(clientSock, lsReturn.size());

      for(int i = 0; i < lsReturn.size(); i++){
        sendString(clientSock, lsReturn[i]);
      }

			cout << endl;
    }
    else if (command == "create")
    {
      getName = receiveString(clientSock);
      cout << getName << endl;
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      sendString(clientSock, to_string(uniqueIDCounter));
      string IPs = "";
      for (int i = 0; i<DataNodeIPs.size();i++){
        string IP = DataNodeIPs[i];
        IPs.append(IP);
        IPs.append(" ");
       sendString(clientSock, IPs);
       sendString(clientSock,DataNodePort);
      }
	  
	long numBlockNames = receiveLong(clientSock);
	vector <string> blockNames;
	for(int i = 0; i < numBlockNames; i++){
	string getName = receiveString(clientSock);
	blockNames.push_back(getName);
	}
	    
      check = create(getName, getPath, blockNames, DataNodeIPs, dirMap);
      sendLong(clientSock, check);
      if(check == 1)
	cout << "Success" << endl;
      else
	cout << "Error" << endl;
      cout << endl;
    }
    else if (command == "stat")
    {
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      //call namenode's stat function here
			cout << endl;
    }
    else if (command == "rmdir")
    {
      getName = receiveString(clientSock);
      getPath = receiveString(clientSock);

      cout << getName << endl;
      cout << getPath << endl;

      check = rmdir(getName, getPath, dirMap);
			sendLong(clientSock, check);
			if(check == 1)
				cout << "Success" << endl;
			else
				cout << "Error" << endl;
      //cout << check << endl;
			cout << endl;
    }
    else if (command == "cat")
    {
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      //call namenode's cat function here
			cout << endl;
    }

  //} //end while
}

bool mkdir(string name, string path, DirHashMap& dirMap){
	bool check = false;
	ContainerObject tempDir;
	tempDir.dirName = name;
	tempDir.dirPath = path;
	check = dirMap.put(path, tempDir);

	size_t found = path.find_last_of("/\\");
	if(found != -1){
		ContainerObject* parent = new ContainerObject();
		string shortPath = path.substr(0,found);
		dirMap.get(shortPath, parent);
		parent->directories.push_back(tempDir);
		dirMap.put(shortPath, *parent);
		ContainerObject* test = new ContainerObject();
		dirMap.get(shortPath, test);
	}

	if(check)
		return true;
	else
		return false;
}

bool rmdir(string name, string path, DirHashMap& dirMap){
	bool check = false;
	ContainerObject* tempDir = new ContainerObject();
	check = dirMap.get(path, tempDir);

	size_t found = path.find_last_of("/\\");
	if(found != -1){
		ContainerObject* parent = new ContainerObject();
		string shortPath = path.substr(0,found);
		dirMap.get(shortPath, parent);
		for(int i = 0; i < parent->directories.size(); i++)
			if(parent->directories[i].dirName == name)
				parent->directories.erase(parent->directories.begin()+i);
		dirMap.put(shortPath, *parent);
	}
  
	if(check){
		dirMap.remove(path);
		return true;
	}
	else
		return false;
}

vector<string> ls(string path, DirHashMap& dirMap)
{
	vector<string> returnList;
	ContainerObject* tempDir = new ContainerObject();
	dirMap.get(path, tempDir);
	for(int i = 0; i < tempDir->directories.size(); i++)
		returnList.push_back(tempDir->directories[i].dirName);
	for(int i = 0; i < tempDir->files.size(); i++)
		returnList.push_back(tempDir->files[i].fileName);
	return returnList;
}

bool create(string name, string path, vector<string> chunkID, vector<string> dataNodeIP, DirHashMap& dirMap){
	bool check = false;
	ContainerObject tempFile;
	tempFile.fileName = name;
	tempFile.filePath = path;
	//Block temp;
        //send ENTIRE set of DataNode IPs to Client to decide where to send
    string DN_IPs = "";
    for (int i = 0; i<dataNodeIP.size(); i++){
      DN_IPs.append(dataNodeIP[i]);
      //now we have a string of DN IP addresses to send over the network into receiveString on Client
    }

	check = dirMap.put(path, tempFile);
	size_t found = path.find_last_of("/\\");
	if(found != -1 && check != false){
		ContainerObject* parent = new ContainerObject();
		string shortPath = path.substr(0,found);
		dirMap.get(shortPath, parent);
		parent->files.push_back(tempFile);
		dirMap.put(shortPath, *parent);
	}
	return check;
}

//send peer list ot all datanodes
void heartbeatThreadTask(){
   
  while(true){
    this_thread::sleep_for(chrono::seconds(60));
    cout<< "1-min heartbeat!\n";
    //first turn vectors into char*
    if(DataNodePort == "0")
        this_thread::sleep_for(chrono::seconds(60));
    unsigned short port = (unsigned short)atoi(DataNodePort.c_str());

    string IPs = "";
    for(int i = 0; i<DataNodeIPs.size(); i++){ 
        IPs.append(DataNodeIPs[i]);
    }
   for(int i = 0; i<DataNodeIPs.size(); i++){ 
       int n = DataNodeIPs[i].length();
       char DN_IP[n+1];
       strcpy(DN_IP, DataNodeIPs[i].c_str()); 
       
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(sock < 0) {
    cout << "THREAD: Error with socket" << endl;
    exit(-1);
   }

  char* IPAddr = const_cast<char *>(DN_IP);

  unsigned long servIP;
  int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
  if (status <= 0) {
    cout << "THREAD: Error with convert dotted decimal address to int" << endl;
    exit(-1);
  }
 
  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = servIP;
  servAddr.sin_port = htons(port);
  int connect_attempts = 0;

  status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(status < 0) {
    while(connect_attempts<3){
      status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
      connect_attempts++;
    }
    if(status < 0) {
    cout << "THREAD: Error with connect to IP: " << DataNodeIPs[i] << endl;
    exit(-1);
  }
  }
    sendHeartbeat(sock, IPs);
    cout << "Closing socket in function main after HeartBeat\n";
    close(sock);
  }
 }
}

void sendHeartbeat(int sock, string IPstring){
//sends a single string list of IPs to DataNode
  sendString(sock, "heartbeat");
  sendString(sock, IPstring);
  cout << "HEARTBEAT CONTENTS: " << IPstring << endl;
  //close(sock);
}

//receive a numeric over the network 
long receiveLong(int clientSock)
{
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















