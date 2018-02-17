//Filename: SUFS Client Program

#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <stdio.h>

using namespace std;

void handleCommand(string cmd);
void ls();
void mkdir(string name);
void rmdir(string name);
void create(string name);
void rm(string name);
void cat(string name);

int main()
{
	string user_command;

	//Welcome message
	cout << endl << endl << endl;
	cout << "Welcome to SUFS!" << endl;
  cout << "Command List: " << endl;
  cout << "mkdir <name> -- Make a directory" << endl;
  cout << "rmdir <name> -- Remove a directory" << endl;
  cout << "ls -- List the contents of the current directory" << endl;
  cout << "create <name> <S3 Object> -- Create a file with S3 Object" << endl;
  cout << "rm <name> -- Remove a file" << endl;
  cout << "cat <name> -- See the contents of a file" << endl;
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
	input[0] != "exit"){
		cout << "Invalid command. Please try again." << endl;
		return;
	}

	//Run certain command, based on first index in vector
	//Also handles error catching
  if(input[0] == "ls"){
		if(input.size() != 1){
			cout << "Error. Invalid command line arguments." << endl;
			return;
		} else {
			ls();
		}
  } else if (input[0] == "mkdir"){
		if(input.size() != 2){
			cout << "Error. Invalid command line arguments." << endl;
			return;
		} else {
			mkdir(input[1]);
		}
  } else if (input[0] == "rmdir") {
		if(input.size() != 2){
			cout << "Error. Invalid command line arguments." << endl;
			return;
		} else {
			rmdir(input[1]);
		}
  } else if (input[0] == "create") {
		if(input.size() != 3){
			cout << "Error. Invalid command line arguments." << endl;
			return;
		} else {
			create(input[1]);
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
  }
}

/*
View contents of directory
*/
void ls()
{
	cout << "List Current Directory:" << endl;
}

/*
Make directory, provide name
*/
void mkdir(string name)
{
	cout << "Made Directory: " << name << endl;
}

/*
Remove Directory
*/
void rmdir(string name)
{
	cout << "Removed Directory: " << name << endl;
}

/*
Create file, provide name, and S3 Object path
*/
void create(string name)
{
	cout << "Created File: " << name << endl;
}

/*
Remove file, provide file name
*/
void rm(string name)
{
	cout << "Removed File: " << name << endl;
}

/*
View contents of file, provided file name
*/
void cat(string name)
{
	cout << "Viewing File Content of: " << name << endl;
}
