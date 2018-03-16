//Filename: SUFS Client Program
//testing if git works
#include <iostream>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <cstdlib>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <algorithm>

using namespace std;
const int REPFACTOR = 3;
//TODO: man 2 sendfile to chunk and move blocks to server EC2
const long chunkSize = 67108864;
int counter = 0;

void handleCommand(string cmd);
void ls(string filepath, int socket);
void mkdir(string name, string path, int socket);
void rmdir(string name, string path, int socket);
void create(string name, string path, string S3_file, string S3_bucket, int socket);
void cat(string path, int socket);
void sendBlock(int sock, string file_name);
void sendLong(int clientSock, long size);
void sendBlockHelper(int sock, string file_name);
void sendString(int sock, string wordSent);
string receiveString(int sock);
long receiveLong(int clientSock);
int chunkFile(string fullFilePath, string chunkName);
void getObject(string s3file, string s3bucket);
void removeFile(string file);
void safeClose(int socket);
int getNNsocket(char*IP, char*port);
void stat(string name, int socket);
char*NameNodeIP;
char*NameNodePort;

int main(int argc, char const *argv[])
{
  srand(time(NULL));
  if(argc < 3) {
    cout << "Error: Missing command line arguments" << endl;
    cout << "Usage: ./Cient [ip_address] [portnumber]" << endl;
    return 1;
  }
  NameNodeIP = const_cast<char *>(argv[1]);
  NameNodePort = const_cast<char *>(argv[2]);

  // int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  // if(sock < 0) {
  //   cout << "Error with socket" << endl;
  //   exit(-1);
  // }

  // char* IPAddr = const_cast<char *>(argv[1]);
  // unsigned short servPort = atoi(argv[2]);

  // unsigned long servIP;
  // int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
  // if (status <= 0) {
  //   cout << "Error with convert dotted decimal address to int" << endl;
  //   exit(-1);
  // }

  // struct sockaddr_in servAddr;
  // servAddr.sin_family = AF_INET; // always AF_INET
  // servAddr.sin_addr.s_addr = servIP;
  // servAddr.sin_port = htons(servPort);
  // status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  // if(status < 0) {
  //   cout << "Error with connect" << endl;
  //   exit(-1);
  // }

  string user_command;

  //Welcome message
  cout << endl << endl << endl;
  cout << "Welcome to SUFS!" << endl;
  cout << "Command List: " << endl;
  cout << "mkdir <name> <path> -- Make a directory" << endl;
  cout << "rmdir <name> <path> -- Remove a directory" << endl;
  cout << "ls <path> -- List the contents of the current directory" << endl;
  cout << "create <name> <path> <s3 filename> <s3 bucket name>-- Create a file with S3 Object" << endl;
  cout << "cat <path> -- See the contents of a file" << endl;
  cout << "stat <path> -- List the DataNodes that store replicas of each block of a file" << endl;
  cout << "exit -- Exit SUFS" << endl;
  cout << endl;

  //Command line input loop
  while(user_command != "exit")
    {
      cout << ">> ";
      getline(cin, user_command);
      handleCommand(user_command);
    }
    sendString(getNNsocket(NameNodeIP, NameNodePort), "exit");
  cout << endl << endl << endl;
  return 0;
}

//Function that handles which command was entered
void handleCommand(string cmd)
{
  int socket = getNNsocket(NameNodeIP,NameNodePort);
  //cout << "SOCKET! " << socket << endl;
  string buf;
  stringstream ss(cmd);
  vector<string> input;

  while (ss >> buf){
    input.push_back(buf);
  }

  //error catch for invalid commands from user
  if(input[0] != "ls" && input[0] != "mkdir" && input[0] != "rmdir" &&
     input[0] != "create" && input[0] != "cat" &&
     input[0] != "exit" && input[0] != "stat"){
    cout << "Invalid command. Please try again." << endl;
    return;
  }

  //Run certain command, based on first index in vector
  //Also handles error catching
  if(input[0] == "ls"){
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
	safeClose(socket);
      return;
    } else {
      ls(input[1], socket);
    }
  } else if (input[0] == "mkdir"){
    if(input.size() != 3){
      cout << "Error. Invalid command line arguments." << endl;
	    safeClose(socket);
      return;
    } else {
      mkdir(input[1], input[2], socket);
    }
  } else if (input[0] == "rmdir") {
    if(input.size() != 3){
      cout << "Error. Invalid command line arguments." << endl;
	    safeClose(socket);
      return;
    } else {
      rmdir(input[1], input[2], socket);
    }
  } else if (input[0] == "create") {
    if(input.size() != 5){
      cout << "Error. Invalid command line arguments." << endl;
	    safeClose(socket);
      return;
    } else {
      create(input[1], input[2], input[3], input[4], socket);
    }
  } else if (input[0] == "cat") {
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
	    safeClose(socket);
      return;
    } else {
      cat(input[1], socket);
    }
  } else if (input[0] == "stat"){
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
	    safeClose(socket);
      return;
    } else {
      stat(input[1], socket);
    }
  }
  safeClose(socket);
}


/*
View contents of directory
Provide absolute filepath
*/
void ls(string filepath, int socket)
{
  sendString(socket, "ls");
  sendString(socket, filepath);

  int numItems = receiveLong(socket);

  if(numItems == 0){
    return;
  }

  cout << "Listing current directory \"" << filepath << "\"" << endl;
  for(int i = 0; i < numItems; i++){
    string getItem = receiveString(socket);
    cout << "-" << getItem << endl;
  }
}

/*
Make directory
Provide directory name and path to where directory will be created
*/
void mkdir(string name, string path, int socket)
{
  sendString(socket, "mkdir");
  sendString(socket, name);
  sendString(socket, path);

  long response = receiveLong(socket);
  if(response == 1)
    cout << "Successfully made directory \"" << name << "\""<< endl;
  else
    cout << "Failed to make directory" << endl;
}

/*
Remove Directory
Provide absolute path to directory to be deleted
Directory must be empty to delete
*/
void rmdir(string name, string path, int socket)
{
  sendString(socket, "rmdir");
  sendString(socket, name);
  sendString(socket, path);

  long response = receiveLong(socket);
  
  if(response == 1)
    cout << "Successfully removed Directory: " << path << endl;
  else
    cout << "Failed to remove directory" << endl;
}

//sendToDataNode(), which takes in an IP, a Port, and a chunked file name
void blockToDataNode(char* DNIPaddr, unsigned short port, string chunkedFile){
  //set up connection to given DataBlock then funnel into sendBlock.
   int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(sock < 0) {
    cout << "blockToDataNode: Error with socket" << endl;
    exit(-1);
   }

  char* IPAddr = const_cast<char *>(DNIPaddr);
  printf("\nSuppossed blockToDataNode IP: %s\n", IPAddr);
  unsigned long servIP;
  int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
  if (status <= 0) {
    cout << "blockTODataNode: Error with convert dotted decimal address to int" << endl;
    exit(-1);
  }

  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = servIP;
  servAddr.sin_port = htons(port);
  status = connect (sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(status < 0) {
    cout << "blockToDataNode: Error with connect" << endl;
    exit(-1);
  }
  sendString(sock, "block");
  sendBlock(sock, chunkedFile); 
 safeClose(sock); 
}

/*
Create file
Provide file name, absolute filepath, S3 Object address
*/
void create(string name, string path, string S3_file, string S3_bucket, int socket)
{
  sendString(socket, "create");
  sendString(socket, name);
  sendString(socket, path);
  
  //Get chunk IDs and data node IPs from Name Node and 
  //process into vectors, then send blocks/chunks to Data Nodes  
  string originalBase = receiveString(socket);
  string baseName = originalBase;
  replace(baseName.begin(), baseName.end(), '/', '_');

  //baseName = "test_test";
  //string DataNodeIPs = receiveString(socket);
  long numIPs = receiveLong(socket);
  //cout << "Suppossedly " << numIPs << " IPs from server";
  vector<string> IPs;
  cout << "getting IPs:" << endl;
  for(int i = 0; i < numIPs; i++){
    string getIP = receiveString(socket);
    IPs.push_back(getIP);
    cout << "getting IP " << getIP << " from Name Node\n";
  }
  cout << "IP RECEIPT CHECK: "<< IPs.size() <<" IPs: \n";
  for(int j = 0; j < IPs.size(); j++){
      cout << IPs[j] << endl;
  }
  //string cleanoutsocket = receiveString(socket);
  //string getStringPort = receiveString(socket);
  string getStringPort (NameNodePort);//"8081";
  cout << "DataNode stats: \n" << "Port: " << getStringPort << endl;
  cout << "Base Name: " << baseName << endl;// << "\nDataNodes to send blocks to: " << IPs << endl;
  
  unsigned short dataNodePort = (unsigned short)stoi(getStringPort);

  vector<string> blockIDnames;
 
  cout << endl;
  for(int i = 0; i < IPs.size(); i++){
    cout << "IPs available: " << IPs[i] << endl;
  }

  //download object from S3
  getObject(S3_file, S3_bucket);
  //chunk the file into 64 MB blocks and return the total num blocks
  cout << "Chunking file..." << endl;
  int numChunks = chunkFile(S3_file, baseName);
  cout << "Num chunks: " << numChunks << endl;

  int numDataNodes = IPs.size();
  cout << "Number of IPs: " << numDataNodes << endl;

  for(int i = 0; i < numChunks; i++){
    string chunkedFileName = originalBase + "." + to_string(i);
    blockIDnames.push_back(chunkedFileName);
  }
  
  cout << "Number of blockIDs: " << blockIDnames.size() << endl;
  sendLong(socket, (long)blockIDnames.size());

  for(int i = 0; i < blockIDnames.size(); i++){
    
    sendString(socket, blockIDnames[i]);
  }

  for(int i = 1; i <= numChunks; i++){
    string chunkedFileName = baseName + "." + to_string(i);
    for (int j = 0; j < REPFACTOR; j++){
      int sendingIP = counter % IPs.size();
      char * IP = const_cast<char*>(IPs[sendingIP].c_str());
      cout << "Sending chunk: " << i << " to node: " << sendingIP;
      blockToDataNode(IP, dataNodePort, chunkedFileName);
      counter++;
    }  
  }
    
  removeFile(S3_file);
  
  for(int i = 1; i <= numChunks; i++){
    string chunkedFileName = baseName + "." + to_string(i);
    removeFile(chunkedFileName);
  }
  cout <<"Done creating file\n";
	
  //determine success or error
  //long response = receiveLong(socket);
  //cout << "RESPONSE STATIS: " << response << endl;
  //if(response == 1)
    //cout << "Successfully created file \"" << name << "\" " << endl;
 // else
   // cout << "Error creating file" << endl;
}

/*
View contents of file
Provide absolute file path
*/
void cat(string path, int socket)
{
  cout << "Viewing File Content of: " << path << endl;
  
  sendString(socket, "cat");
  //send the absolute to namenode data structure 
  sendString(socket, path);
  long numBlocks = receiveLong(socket);
  cout << "about to request" << numBlocks << " blocks " << "for " << path << endl;
  std::vector<string> blockNames;
  std::vector<string> IPs;
  for (int i = 0; i<numBlocks; i++){
      string block = receiveString(socket);
      blockNames.push_back(block);
      cout << "block: " << block << endl;
  }  
  for (int j = 0; j<numBlocks; j++){
    string IP = receiveString(socket);
    IPs.push_back(IP);
    cout << "received IP " << IP << endl; 
  }
  for(int k = 0; k<numBlocks; k++){
    char*IP = const_cast<char *>(IPs[k].c_str());
    int socket = getNNsocket(IP, NameNodePort);
    sendString(socket, blockNames[k]);
  }
  close(socket);
  //GET datanode socket
  //
  //will be receiving block_IDs and associated DataNode_IPs
  //use information from stat? 
  //create stat first before writing cat
  //get all the block_IDs and DataNode_IPs

}

/*
View stat of file
Provide filename
*/
void stat(string name, int socket)
{
  cout << "Stat Content of: " << name << endl;
  sendString(socket, "stat");
  sendString(socket, name);
  
  long numStats = receiveLong(socket);
  cout << "num stats: " << numStats << endl;

  for(int i = 0; i < numStats; i++){
    string chunkID = receiveString(socket);
    cout << chunkID << " ";
    int numIPs = receiveLong(socket);
    for(int j = 0; j < numIPs; j++){
      string IP = receiveString(socket);
      cout << IP << " ";
    }
    cout << endl;
  }
}

/*
chunk File into multiple blocks
*/
int chunkFile(string fullFilePath, string chunkName)
{
    ifstream fileStream;
    fileStream.open(fullFilePath.c_str(), ios::in | ios::binary);
    //cout << "opened file" << endl;

    // File open a success
    if (fileStream.is_open()) {
      //cout << "open success" << endl;  
      ofstream output;
        int counter = 1;
        string fullChunkName;
        // Create a buffer to hold each chunk
        char *buffer = new char[chunkSize];
        // Keep reading until end of file
        while (!fileStream.eof()) {
        // Build the chunk file name. Usually drive:\\chunkName.ext.N
           // N represents the Nth chunk
            fullChunkName.clear();
            fullChunkName.append(chunkName);
            fullChunkName.append(".");
	    // cout << "just built chunk names" << endl;
           // Convert counter integer into string and append to name.
             string intBuf = to_string(counter);
	     fullChunkName.append(intBuf);
           // Open new chunk file name for output
            output.open(fullChunkName.c_str(),ios::out | ios::trunc | ios::binary);
            // If chunk file opened successfully, read from input and
            // write to output chunk. Then close.
	    //cout <<"write to output chunk" << endl;
	     if (output.is_open()) {
	       // cout <<"inner if statement" << endl; 
	       fileStream.read(buffer,chunkSize);
                // gcount() returns number of bytes read from stream.
                output.write(buffer,fileStream.gcount());
                output.close();

                counter++;
            }
        }
        // Cleanup buffer
        delete[] buffer;

        // Close input file stream.
        fileStream.close();
        cout << "Chunking complete! " << counter - 1 << " files created." << endl;
	return (counter - 1);
    }
    else { cout << "Error opening file!" << endl; }
    return 0;
}

/*
Should specify filename and bucket_name
Right now, for testing purposing, it's getting from Tu Trinh's sufs-test bucket
*/
void getObject(string s3file, string s3bucket)
{
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {
    Aws::String FILE = Aws::Utils::StringUtils::to_string(s3file);
    Aws::String BUCKET = Aws::Utils::StringUtils::to_string(s3bucket);
    const Aws::String bucket_name = BUCKET;
    const Aws::String key_name = FILE;

    std::cout << "Downloading " << key_name << " from S3 bucket: " <<
      bucket_name << std::endl;

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::US_WEST_2;
    Aws::S3::S3Client s3_client(clientConfig);

    Aws::S3::Model::GetObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto get_object_outcome = s3_client.GetObject(object_request);

    if (get_object_outcome.IsSuccess())
      {
	Aws::OFStream local_file;
	local_file.open(key_name.c_str(), std::ios::out | std::ios::binary);
	local_file << get_object_outcome.GetResult().GetBody().rdbuf();
	std::cout << "Get Success!" << std::endl;
      }
    else
      {
	std::cout << "GetObject error: " <<
	  get_object_outcome.GetError().GetExceptionName() << " " <<
	  get_object_outcome.GetError().GetMessage() << std::endl;
      }
  }
  Aws::ShutdownAPI(options);
}

//send message/data over the network 
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

//recieve message/data 
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

//send a numeric over the network 
void sendLong(int clientSock, long size)
{
  size = htonl(size);
  int bytesSent = send(clientSock, (void *) &size, sizeof(long), 0);
  if(bytesSent != sizeof(long)) {
    pthread_exit(NULL);
  }
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

//C++-based: takes client socket and block file name and
//then sends name and size to sendBlockHelper to send
void sendBlock(int sock, string file_name){

    sendString(sock, file_name);
    //now take file size and send over
    FILE* readPtr;
    readPtr = fopen(file_name.c_str(),"rb");
    cout << "Writing to file: " << file_name << endl;
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
      //cout << "sent " << bytesSent << " remain " << remaining_to_send << "\n";
    }

      fclose(readPtr);
}

//delete a file after sending
void removeFile(string file)
{
  string filename = file;
  const char* deleteFile = filename.c_str();

  if(remove(deleteFile) != 0)
    perror("Error deleting file");
  else
    puts("File successfully deleted");
}


void safeClose(int socket){
  if(close(socket) != 0)
    return;
}

int getNNsocket(char*IP, char*port){
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock < 0) {
    cout << "Error with socket" << endl;
    exit(-1);
  }

  char* IPAddr = const_cast<char *>(IP);
  unsigned short servPort = atoi(port);

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
  return sock;
}
