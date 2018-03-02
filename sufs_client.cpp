//Filename: SUFS Client Program

#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <stdio.h>

using namespace std;

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

/*
** Client - NameNode Functions?
*/

/*
** Client - DataNode Functions?
*/


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

/*
*******************************************************************************
*/

/*
View contents of directory
Provide absolute filepath
*/
void ls(string filepath)
{

  cout << "List Current Directory: " << filepath << endl;
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
