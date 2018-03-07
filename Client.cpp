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

#define PORT 8080
using namespace std;

//TODO: man 2 sendfile to chunk and move blocks to server EC2

/*
** Client Functions
These function help with starting/handling the commands typed in my user
*/
void handleCommand(string cmd);
void ls(string filepath);
void mkdir(string name, string path);
void rmdir(string path);
void create(string name, string path, string S3_address);
void rm(string path);
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
  cout << "create <name> <path> <S3 Object> -- Create a file with S3 Object" << endl;
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
    if(input.size() != 4){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      create(input[1], input[2], input[3]);
    }
  } else if (input[0] == "rm") {
    if(input.size() != 2){
      cout << "Error. Invalid command line arguments." << endl;
      return;
    } else {
      rm(input[1]);
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
    if(inet_pton(AF_INET, "10.124.72.20", &serv_addr.sin_addr)<=0) 
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
}

/*
Make directory
Provide directory name and path to where directory will be created
*/
void mkdir(string name, string path)
{
  cout << "Made Directory: " << name << endl;
}

/*
Remove Directory
Provide absolute path to directory to be deleted
Directory must be empty to delete
*/
void rmdir(string path)
{
  cout << "Removed Directory: " << path << endl;
}

/*
Create file
Provide file name, absolute filepath, S3 Object address
*/
void create(string name, string path, string S3_address)
{
  cout << "Created File: " << name << endl;
}

/*
Remove file
Provide absolute file path
*/
void rm(string path)
{
  cout << "Removed File: " << path << endl;
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
