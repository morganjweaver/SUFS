//Filename: SUFS Client Program

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

#define PORT 8080
using namespace std;

//TODO: man 2 sendfile to chunk and move blocks to server EC2
const long chunkSize = 67108864;

const int SUCCESS = 0;
const int FILE_NOT_EXIST = 1;
const int PATH_NOT_EXIST = 2;
const int FILE_EXISTS = 3;
const int DIRECTORY_EXIST = 4;
const int DIRECTORY_NOT_EMPTY = 5;

/*
** Client Functions
These function help with starting/handling the commands typed in my user
*/
void handleCommand(string cmd, int socket);
void ls(string filepath, int socket);
void mkdir(string name, string path, int socket);
void rmdir(string path, int socket);
void create(string name, string path, string S3_file, string S3_bucket, int socket);
void cat(string path, int socket);
void stat(string name, int socket);
void sendBlock(int sock, string file_name);
void sendLong(int clientSock, long size);
void sendBlockHelper(int sock, string file_name);
/*
** Client - NameNode Functions?
*/
void sendString(int sock, string wordSent);
string receiveString(int sock);

/*
** Client - DataNode Functions?
*/

//char *sendRPC(char* request);
void chunkFile(string fullFilePath, string chunkName);
void getObject(string s3file, string s3bucket);

int main(int argc, char const *argv[])
{
  if(argc < 3) {
    cout << "Error: Missing command line arguments" << endl;
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

  string user_command;

  //Welcome message
  cout << endl << endl << endl;
  cout << "Welcome to SUFS!" << endl;
  cout << "Command List: " << endl;
  cout << "mkdir <name> <path> -- Make a directory" << endl;
  cout << "rmdir <path> -- Remove a directory" << endl;
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
      handleCommand(user_command, sock);
    }
    sendString(sock, "exit");
  cout << endl << endl << endl;
  return 0;
}

//Function that handles which command was entered
void handleCommand(string cmd, int socket)
{
  string buf;
  stringstream ss(cmd);
  vector<string> input;
  while (ss >> buf){
    input.push_back(buf);
  }

  //error catch for invalid commands from user
  if(input[0] != "ls" && input[0] != "mkdir" && input[0] != "rmdir" &&
     input[0] != "create" && input[0] != "rm" && input[0] != "cat" &&
     input[0] != "exit" && input[0] != "stat"){
    cout << "Invalid command. Please try again." << endl;
    return;
  }

  //Run certain command, based on first index in vector
  //Also handles error catching
  if(input[0] == "ls"){
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      ls(input[1], socket);
    }
  } else if (input[0] == "mkdir"){
    if(input.size() != 3){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      mkdir(input[1], input[2], socket);
    }
  } else if (input[0] == "rmdir") {
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      rmdir(input[1], socket);
    }
  } else if (input[0] == "create") {
    if(input.size() != 5){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      create(input[1], input[2], input[3], input[4], socket);
    }
  } else if (input[0] == "cat") {
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      cat(input[1], socket);
    }
  } else if (input[0] == "stat"){
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      stat(input[1], socket);
    }
  }

}


/*
View contents of directory
Provide absolute filepath
*/
void ls(string filepath, int socket)
{

  cout << "List Current Directory: " << filepath << endl;
  sendString(socket, "ls");
  sendString(socket, filepath);

}

/*
Make directory
Provide directory name and path to where directory will be created
*/
void mkdir(string name, string path, int socket)
{
  cout << "Made Directory: " << name << endl;
  sendString(socket, "mkdir");
  sendString(socket, name);
  sendString(socket, path);
}

/*
Remove Directory
Provide absolute path to directory to be deleted
Directory must be empty to delete
*/
void rmdir(string path, int socket)
{
  cout << "Removed Directory: " << path << endl;
  sendString(socket, "rmdir");
  sendString(socket, path);
}

/*
Create file
Provide file name, absolute filepath, S3 Object address
*/
void create(string name, string path, string S3_file, string S3_bucket, int socket)
{
	sendBlock(socket, "sendme.txt");
  cout << "Created File: " << name << endl;
  //sendString(socket, "create");
  //sendString(socket, S3_file);
  //sendString(socket, path);

	
  //socket code here
  //sent to nameNode file creation request
  //nn_create(name, path);

  //wait for response
  //error or success

  //get from NameNode some DataNode information

  //get the file from S3 into local drive
  //getObject(S3_file, S3_bucket);

  //chunkFile(S3_file, name);
  //get num chunks returned from chunkFile
  /*
  int dnode_id = 1;
  for(int i = 1; i <= numChunks; i++){
    sendChunk(name + "." + i, dnode + dnode_id)
    dnode_id++;
    if(dnode_id == 4)
      dnode_id = 1;
  }
  */
}

/*
View contents of file
Provide absolute file path
*/
void cat(string path, int socket)
{
  cout << "Viewing File Content of: " << path << endl;
  sendString(socket, "cat");
  sendString(socket, path);
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
}

/*
chunk File into multiple blocks
*/
void chunkFile(string fullFilePath, string chunkName)
{
    ifstream fileStream;

    fileStream.open(fullFilePath.c_str(), ios::in | ios::binary);

    // File open a success

    if (fileStream.is_open()) {

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
           // Convert counter integer into string and append to name.
             string intBuf = to_string(counter);
	     fullChunkName.append(intBuf);
           // Open new chunk file name for output
            output.open(fullChunkName.c_str(),ios::out | ios::trunc | ios::binary);
            // If chunk file opened successfully, read from input and
            // write to output chunk. Then close.

	     if (output.is_open()) {
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
    }
    else { cout << "Error opening file!" << endl; }
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
void sendLong(int clientSock, long size) {
  size = htonl(size);
  int bytesSent = send(clientSock, (void *) &size, sizeof(long), 0);
  if(bytesSent != sizeof(long)) {
    pthread_exit(NULL);
  }
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
    close(readPtr);
    free(readPtr);
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
      close(readPtr);
      free(readPtr);
    }
}
