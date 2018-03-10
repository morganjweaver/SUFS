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
#include "DirHashMap.cpp"
#include "NodeHashMap.cpp"
#define PORT 8080

using namespace std;

//Error messages to send back to client 
const int SUCCESS = 0;
const int FILE_NOT_EXIST = 1;
const int PATH_NOT_EXIST = 2;
const int FILE_EXISTS = 3;
const int DIRECTORY_EXIST = 4;
const int DIRECTORY_NOT_EMPTY = 5;
const int DIRECTORY_NOT_EXIST = 6;
const int MAXPENDING = 99;

void processClient(int new_client_socket);
void sendString(int sock, string wordSent);
string receiveString(int sock);
void processClient(int clientSock, string clientIP);
void processHeartbeat(string nodeIPaddr, string heartbeat_data);

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

vector<string> ls(string path, DirHashMap& dirMap){
	vector<string> returnList;
	ContainerObject* tempDir = new ContainerObject();
	dirMap.get(path, tempDir);
	for(int i = 0; i < tempDir->directories.size(); i++)
		returnList.push_back(tempDir->directories[i].dirName);
	for(int i = 0; i < tempDir->files.size(); i++)
		returnList.push_back(tempDir->files[i].fileName);
	return returnList;
}

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
//add heartbeat information to DataNode:block hashtable

void processHeartbeat(string nodeIPaddr, string heartbeat_data){
    //if put attempt returns false, remove the entry and try again
  
  //DATA_NODE_HASHMAP_HERE
  vector<string> blockFileNames;
  string fileName;
  stringstream s (heartbeat_data);
  while(s>> fileName)
    blockFileNames.push_back(fileName);
  
  // Now we have a vector of block file IDs and the IP addr of the DataNode that holds them
  // Add global hashmap fof block-->vector<string file> table here!!

  // if(HASHMAP.put(nodeIPaddr, blockFileNames) == false){
  //   cout << "failed to put addresses" << endl;
  //   exit(-1);
  // }

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
  while(true)
  {
    command = receiveString(clientSock);
    cout << command << endl;
    
    //if command is a write block to file, add recieveBlock
    
    //if client exits, close the server 
    if(command == "exit"){
      close(clientSock);
      exit(-1);
    }
    if(command == "heartbeat"){
      string blocks = receiveString(clientSock);
      processHeartbeat(blocks, clientIP);
    }
    if(command == "mkdir")
    {
      getName = receiveString(clientSock);
      cout << getName << endl;
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      check = mkdir(getName, getPath, dirMap);
      cout << check << endl;
    } 
    else if(command == "ls") 
    {
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      lsReturn = ls(getPath, dirMap);
    } 
    else if (command == "create") 
    {
      getName = receiveString(clientSock);
      cout << getName << endl;
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      //call namenode's create function here
    } 
    else if (command == "stat") 
    {
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      //call namenode's stat function here 
    } 
    else if (command == "rmdir")
    {
      getName = receiveString(clientSock);
      cout << getName << endl;
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      check = rmdir(getName, getPath, dirMap);
      cout << check << endl;
    } 
    else if (command == "cat")
    {
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      //call namenode's cat function here
    }
    
  } //end while
}
