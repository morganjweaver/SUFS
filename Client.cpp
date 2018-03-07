//Filename: SUFS Client Program

#include <iostream>
#include <cstring>
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
=======
const long chunkSize = 67108864;

/*
** Client Functions
These function help with starting/handling the commands typed in my user
*/
void handleCommand(string cmd);
void ls(string filepath);
void mkdir(string name, string path);
void rmdir(string path);
void create(string name, string path, string S3_file, string S3_bucket);
void cat(string path);
void stat(string name);
char *sendRPC(char* request);
/*
** Client - NameNode Functions?
*/

/*
** Client - DataNode Functions?
*/

char *sendRPC(char* request);
=======
void chunkFile(string fullFilePath, string chunkName);
int sendRPC(char* request);
void getObject(string s3file, string s3bucket);

int main()
{
  string user_command;

  //Welcome message
  cout << endl << endl << endl;
  cout << "Welcome to SUFS!" << endl;
  cout << "Command List: " << endl;
  cout << "mkdir <name> <path> -- Make a directory" << endl;
  cout << "rmdir <path> -- Remove a directory" << endl;
  cout << "ls <path> -- List the contents of the current directory" << endl;
  cout << "create <name> <path> <s3 filename> <s3 bucket name>-- Create a file with S3 Object" << endl;
  cout << "rm <path> -- Remove a file" << endl;
  cout << "cat <path> -- See the contents of a file" << endl;
  cout << "stat <name> -- See DataNode & Block Replicas" << endl;
  cout << "exit -- Exit SUFS" << endl;
  cout << endl;

  //Command line input loop
  while(user_command != "exit")
    {
      cout << ">> ";
      getline(cin, user_command);
      handleCommand(user_command);
    }

  cout << endl << endl << endl;
  return 0;
}

//Function that handles which command was entered
void handleCommand(string cmd)
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
      ls(input[1]);
    }
  } else if (input[0] == "mkdir"){
    if(input.size() != 3){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      mkdir(input[1], input[2]);
    }
  } else if (input[0] == "rmdir") {
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      rmdir(input[1]);
    }
  } else if (input[0] == "create") {
    if(input.size() != 5){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      create(input[1], input[2], input[3], input[4]);
    }
  } else if (input[0] == "cat") {
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      cat(input[1]);
    }
  } else if (input[0] == "stat"){
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      stat(input[1]);
    }
  }

}

char *sendRPC(char* request){

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char tmp;
    int buflen = 0;
    char err[] = "error";
    char* errptr = err;
    char delimiter[] = "~!!";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return errptr;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    //**SERVER IP ADDR GOES HERE; current is CS1**
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "172.31.21.99", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return errptr;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return errptr;
    }

    printf("INPUT FROM LS: %s\n", request);
    send(sock , request , strlen(request) , 0 );
    printf("Message sent\n");
   
    do
    {
        valread = read(sock, &tmp, 1);
        if (valread < 0) cout<<"ERROR reading from socket";

        /*char * comparer;
        comparer = (char*) memchr((void*)buffer, '~', 1024 );
        if (comparer==NULL){
            cout<<"no delim yet, long string\n";
             continue;
         } else{
              cout<<"FOUND DELIM in first buffer read\n";
              char *string =(char*) (malloc(1024*sizeof(char)));
              strncpy(string,buffer,1024 );
              return string;
          }*/
         if (tmp != '~')
             continue;
         buflen = 0;
     do{
        
        valread = read( sock , &tmp, 1);
        if (valread  < 0){
         cout<<"ERROR reading from socket" << endl;
        }
        /*char * comparer;
        comparer = (char*) memchr ((void*)&tmp, '~!!', 3 );
        if (comparer!=NULL){
            cout<<"COMPARISON SUCCESS\n";
            break;
         }*/
        if(tmp=='~')
            break;
         else{
         
         // TODO: if the buffer's capacity has been reached, either reallocate the buffer with a larger size, or fail the operation...
        buffer[buflen] = tmp;
        ++buflen;
        }}
	while(1);
        
        char *string =(char*) (malloc(buflen*sizeof(char)));
        strncpy(string,buffer, buflen );
        return string;
        printf("%*.*s\n", buflen, buflen, buffer);

} while(1);
}
/*
*******************************************************************************
*/

/*
View contents of directory
Provide absolute filepath
*/
void ls(string filepath)
{
  string temp = filepath;
  const char* request = temp.c_str();
  cout << "List Current Directory: " << filepath << endl;
  char* handled = sendRPC(const_cast<char*>(request));
  cout<< "Printing output: \n";
  printf("%s\n",handled);
  if(strcmp(handled,"error")!=0){
    cout<<"SUCCESS ls function and 0 return of RPC fx \n";
  } else{
      cout<<"NO SUCCESS on ls fx and RPC fx\n";
  }

  cout << "List Current Directory: " << filepath << endl;
}

/*
Make directory
Provide directory name and path to where directory will be created
*/
void mkdir(string name, string path)
{
  string filename = name;
  string abspath = path;
  const char* request_name = name.c_str();
  const char* request_path = path.c_str();

  //send the directory name
  int handled = sendRPC(const_cast<char*>(request_name));
  if(handled == 0){
    cout<<"SUCCESS mkdir function and 0 return of RPC fx" << endl;;
  } else{
      cout<<"NO SUCCESS on mkdir fx and RPC fx" << endl;;
  }

  //send the path to where the directory should be placed
  handled = sendRPC(const_cast<char*>(request_path));
  if(handled == 0){
    cout<<"SUCCESS mkdir function and 0 return of RPC fx" << endl;
  } else{
      cout<<"NO SUCCESS on mkdir fx and RPC fx" << endl;
  }

  cout << "Made Directory: " << name << endl;
}

/*
Remove Directory
Provide absolute path to directory to be deleted
Directory must be empty to delete
*/
void rmdir(string path)
{
  string temp = path;
  const char* request = temp.c_str();

  int handled = sendRPC(const_cast<char*>(request));
  if(handled == 0){
    cout<<"SUCCESS rmdir function and 0 return of RPC fx" << endl;
  } else{
      cout<<"NO SUCCESS on rmdir fx and RPC fx" << endl;
  }

  cout << "Removed Directory: " << path << endl;
}

/*
Create file
Provide file name, absolute filepath, S3 Object address
*/
void create(string name, string path, string S3_file, string S3_bucket)
{
  cout << "Created File: " << name << endl;

  //socket code here
  //sent to nameNode file creation request
  //nn_create(name, path);

  //wait for response
  //error or success

  //get from NameNode some DataNode information

  //get the file from S3 into local drive
  getObject(S3_file, S3_bucket);

  chunkFile(S3_file, name);
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
void cat(string path)
{
  cout << "Viewing File Content of: " << path << endl;
}

/*
View stat of file
Provide filename
*/
void stat(string name)
{
  cout << "Stat Contenet of: " << name << endl;
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
