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
#include <cxxabi.h>
#include <stdio.h>
#include <thread>
#include <exception>
#include "DirHashMap.cpp"
#include "IPHashMap.cpp"
#include "ChunkHashMap.cpp"
#include "StatObject.cpp"
#include "CatObject.cpp"

DirHashMap dirMap;
IPHashMap IPMap;
ChunkHashMap chunkMap;

using namespace std;

//Error messages to send back to client

const int MAXPENDING = 99;

void processClient(int new_client_socket);
void sendString(int sock, string wordSent);
string receiveString(int sock);
void processClient(int clientSock, string clientIP);
void processHeartbeat(string clientPort, string nodeIPaddr, string heartbeat_data);
bool mkdir(string name, string path);
bool rmdir(string name, string path);
vector<string> ls(string path);
vector<string> DataNodeIPs;
void heartbeatThreadTask();
void sendHeartbeat(int sock, string IPstring);
bool create(string name, string path, vector<string> chunkID, vector<string> dataNodeIP);
vector<StatObject> stat(string path);
vector<CatObject> cat(string path);
long receiveLong(int clientSock);

string DataNodePort = "8080";
//SERVER SOCKET CODE

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
    //DataNodeIPs.push_back("172.31.19.92");
    while(true){
      try{
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
	size_t tempSize = sizeof(ipstr);
	ipstr[tempSize] = '\0';

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
        cout << "Processing client connection from: " << ipstr << endl;
        processClient(clientSock, ipstr);        
        close(clientSock);
      }
      catch(const std::runtime_error& re) {
          // speciffic handling for runtime_error
          std::cerr << "Runtime error: " << re.what() << std::endl;
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

void processHeartbeat(string clientPort, string nodeIPaddr, string heartbeat_data){
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
	stringstream s(heartbeat_data);
	istream_iterator<string> begin(s);
	istream_iterator<string> end;
	vector<string> blockFileNames(begin, end);
	for(int i = 0; i < blockFileNames.size(); i++){
		size_t found = blockFileNames[i].find_last_of(".");
		string filePath = blockFileNames[i].substr(0,found);
		ContainerObject* myFile = new ContainerObject();
		dirMap.get(filePath, myFile);
		myFile->blocks.clear();
		dirMap.put(filePath, *myFile);
	}	
	for(int i = 0; i < blockFileNames.size(); i++){
		size_t found = blockFileNames[i].find_last_of(".");
		string filePath = blockFileNames[i].substr(0,found);
		ContainerObject* myFile = new ContainerObject();
		dirMap.get(filePath, myFile);
		Block tempBlock;
		tempBlock.chunk_ID = blockFileNames[i];
		tempBlock.IP = nodeIPaddr;
		for(int j = 0; j < myFile->blocks.size(); j++)
			if(myFile->blocks[j].chunk_ID == blockFileNames[i])
				continue;
		myFile->blocks.push_back(tempBlock);
		dirMap.put(filePath, *myFile);
		if(chunkMap.put(blockFileNames[i], nodeIPaddr) == false){
			cout << "failed to put addresses" << endl;
			exit(-1);
		}
	}
	
  if(IPMap.put(nodeIPaddr, blockFileNames) == false){
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
  vector<string> lsReturn;
  bool check = false;
  //while(true)
  //{
    cout << "waiting for command" << endl;
    command = receiveString(clientSock);
    cout << "command: " << command << endl;
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

      processHeartbeat(port, clientIP, blocks);
      //close(clientSock);
    }

    if(command == "mkdir")
    {
      getName = receiveString(clientSock);
      cout << getName << endl;
      getPath = receiveString(clientSock);
      cout << getPath << endl;
      check = mkdir(getName, getPath);
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
        lsReturn = ls(getPath);
      
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
      	sendString(clientSock, getPath);
        cout << "\n\nNumber of DN IPS: " << DataNodeIPs.size() << endl;
      	sendLong(clientSock, (DataNodeIPs.size()));
        //cout << "About to send to client " << DataNodeIPs.size() << " IPs of DNs\n";
      	for(int i = 0; i < DataNodeIPs.size(); i++){
      	  sendString(clientSock, DataNodeIPs[i]);
	        cout << "sending IP " << DataNodeIPs[i] << "\n";
        }
  //cout << "sending port " << DataNodePort << endl;
	//sendString(clientSock, DataNodePort);
	
	cout << "receiving blocks" << endl;
	// OH NOES
	long numBlockNames = receiveLong(clientSock);
	cout << "Number of blocks: " << numBlockNames << endl; 
	vector <string> blockNames;
	for(int i = 0; i < numBlockNames; i++){
	  string getName = receiveString(clientSock);
        cout << "receiving name : " << getName << endl;
	blockNames.push_back(getName);
      }
      
      cout << "creating file" << endl;
      check = create(getName, getPath, blockNames, DataNodeIPs);
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
      vector<StatObject> myStats;
      myStats = stat(getPath);
      vector<string> statInfo;
      for(int i = 0; i < myStats.size(); i++){
	      string tempstr = myStats[i].chunk_ID + ": ";
	      for(int j = 0; j < myStats[i].repIP.size(); j++){
		      myStats[i].repIP[j][myStats[i].repIP[j].length()] = '\0';
		      tempstr = tempstr + " " + myStats[i].repIP[j] ;
	      }
	      statInfo.push_back(tempstr);
      }
      for(int i = 0; i < statInfo.size(); i++){
	      cout << statInfo[i] << endl;
      }
      sendLong(clientSock, statInfo.size());
      for(int i = 0; i < statInfo.size(); i++){
        sendString(clientSock, statInfo[i]);
      }
      cout << endl;
    }
    else if (command == "rmdir")
    {
      getName = receiveString(clientSock);
      getPath = receiveString(clientSock);

      cout << getName << endl;
      cout << getPath << endl;

      check = rmdir(getName, getPath);
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
      	vector<CatObject> catFile;
      	catFile = cat(getPath);
      	sendLong(clientSock, catFile.size());
      	for(int i = 0; i < catFile.size(); i++)
      		sendString(clientSock, catFile[i].chunk_ID);
      	sendLong(clientSock, catFile.size());
      	for(int i = 0; i < catFile.size(); i++)
      		sendString(clientSock, catFile[i].IP);
      	cout << endl;
    }

  //} //end while
}

bool mkdir(string name, string path){
	bool check = false;
	bool checkParent = false;
	ContainerObject tempDir;
	tempDir.dirName = name;
	tempDir.dirPath = path;

	size_t found = path.find_last_of("/\\");
	if(found != -1){
		ContainerObject* parent = new ContainerObject();
		string shortPath = path.substr(0,found);
		checkParent = dirMap.get(shortPath, parent);
		if(checkParent == false){
			check = false;
			return check;
		}
		dirMap.put(path, tempDir);
		parent->directories.push_back(tempDir);
		dirMap.put(shortPath, *parent);
		check = true;
	}
	dirMap.put(path, tempDir);
	check = true;
	return check;
}

bool rmdir(string name, string path){
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

vector<string> ls(string path)
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

bool create(string name, string path, vector<string> chunkID, vector<string> dataNodeIP){
	bool check = false;
	bool checkParent = false;
	ContainerObject tempFile;
	tempFile.fileName = name;
	tempFile.filePath = path;
	Block temp;
	
	size_t found = path.find_last_of("/\\");
	ContainerObject* parent = new ContainerObject();
	string shortPath = path.substr(0,found);
	checkParent = dirMap.get(shortPath, parent);
	if(checkParent == false){
		check = false;
		return check;
	}
	else if(found != -1){
		parent->files.push_back(tempFile);
		dirMap.put(shortPath, *parent);
	}
	check = dirMap.put(path, tempFile);
	return check;
}


vector<StatObject> stat(string path){
	ContainerObject* tempFile = new ContainerObject();
	vector<string> holdIP;
	vector<StatObject> tempStat;
	StatObject holdChunk;
	dirMap.get(path, tempFile);
	for(int i = 0; i < tempFile->blocks.size(); i++){
		chunkMap.get(tempFile->blocks[i].chunk_ID, holdIP);
		holdChunk.chunk_ID = tempFile->blocks[i].chunk_ID;
		holdChunk.repIP = holdIP;
		tempStat.push_back(holdChunk);
	}
	return tempStat;
}

vector<CatObject> cat(string path){
	ContainerObject* tempFile = new ContainerObject();
	vector<CatObject> holdCat;
	vector<string> holdIP;
	CatObject cat;
	dirMap.get(path, tempFile);
	for(int i = 0; i < tempFile->blocks.size(); i++){
		cat.chunk_ID = tempFile->blocks[i].chunk_ID;
		chunkMap.get(cat.chunk_ID, holdIP);
		cat.IP = holdIP[0];
		holdCat.push_back(cat);
	}
	return holdCat;
}
void sendReplicaToPeer(string filename, string DN_IP_ADDR)
//tell a DataNode to send a copy of a specified file to a peer
{
  int n = DN_IP_ADDR.length();
  char DN_IP[n+1];
  strcpy(DN_IP, DN_IP_ADDR.c_str());
  char* IPAddr = const_cast<char *>(DN_IP);
   int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(sock < 0) {
    cout << "THREAD: Error with socket" << endl;
    exit(-1);
   }
  unsigned long servIP;
  int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
  if (status <= 0) {
    cout << "REPLICATE: Error with convert dotted decimal address to int" << endl;
    exit(-1);
  }
  unsigned short port = (unsigned short)stoi(DataNodePort);
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
    stringstream err;
    err << "REPLICATE: Error with connect to IP: " << DN_IP_ADDR << endl;
    throw NetException(err.str());
  }
  }
  //now we have a connection!
  sendString(port, "replicate");
  sendString(port, filename);
  close(sock);
  }


void heartbeatThreadTask(){
   
  while(true){
    try{
    this_thread::sleep_for(chrono::seconds(15));
    cout<< "1-min heartbeat!\n";
    //first turn vectors into char*
    if(DataNodePort == "0")
        this_thread::sleep_for(chrono::seconds(30));
    unsigned short port = (unsigned short)atoi(DataNodePort.c_str());

   for(int i = 0; i<DataNodeIPs.size(); i++){ 
       int n = DataNodeIPs[i].length();
       char DN_IP[n+1];
       strcpy(DN_IP, DataNodeIPs[i].c_str()); 
    string IPs = "";
    for(int j = 0; j<DataNodeIPs.size(); j++){ 
        if(DataNodeIPs[j] != DataNodeIPs[i])
        IPs.append(DataNodeIPs[j]);
        IPs.append(" ");
    } //send each DataNode a list of PEER IPs, not self
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
    stringstream err;
    err << "THREAD: Error with connect to IP: " << DataNodeIPs[i] << endl;
    throw NetException(err.str());
  }
  }
    sendHeartbeat(sock, IPs);
    close(sock);
  }
 }
  catch(const std::runtime_error& re) {
          // speciffic handling for runtime_error
          std::cerr << "Runtime error: " << re.what() << std::endl;
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
}

void sendHeartbeat(int sock, string IPstring){
//sends a single string list of IPs to DataNode
  sendString(sock, "heartbeat");
  sendString(sock, IPstring);
  cout << "HEARTBEAT CONTENTS: " << IPstring << "TO A DATANODE" <<endl;
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
















