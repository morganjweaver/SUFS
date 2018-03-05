//Filename: SUFS Client Program

#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <stdio.h>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <iostream>
#include <fstream>
#include <sstream>

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
void putObject();
void getObject(string file);

//Global vars
int filecount;

int main()
{
  string user_command;
  filecount = 0;

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
  putS3();
  filecount++;
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
  gets3(path);
}

/*
View stat of file
Provide filename
*/
void stat(string name)
{
  cout << "Stat Content of: " << name << endl;
}

/*
Put an object to S3
Current bucket is "sufs-test" on Tu Trinh's aws account
//file should be passed in? or content of file
//Set key_name and file_name to specify file to be created
//set requestStream to load content into file
//use this with chunker?
//also use gets3
*/
void putObject()
{
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {
    //use this line to convert types into Aws::String
    Aws::String NUM = Aws::Utils::StringUtils::to_string(filecount);
    //specify these when loading object onto s3 bucket
    const Aws::String bucket_name = "sufs-test";
    const Aws::String key_name = "testfile" + NUM;
    const Aws::String file_name = "testfile" + NUM;

    //print file details
    std::cout << "Filename: " << file_name << std::endl;
    std::cout << "Key Name: " << key_name << std::endl;
    std::cout << "Bucket Name: " << bucket_name << std::endl;

    //configure region
    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::US_WEST_2;
    Aws::S3::S3Client s3_client(clientConfig);

    Aws::S3::Model::PutObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto requestStream  = Aws::MakeShared<Aws::StringStream>("s3");
    //Data should be loaded into this *requestStream
    *requestStream << "Hello World! Please work!";

    object_request.SetBody(requestStream);

    auto put_object_outcome = s3_client.PutObject(object_request);

    if (put_object_outcome.IsSuccess())
      {
  std::cout << "Put Success!" << std::endl;
      }
    else
      {
  std::cout << "PutObject error: " <<
    put_object_outcome.GetError().GetExceptionName() << " " <<
    put_object_outcome.GetError().GetMessage() << std::endl;
      }
  }
  Aws::ShutdownAPI(options);
}

/*
Should specify filename and bucket_name
Right now, for testing purposing, it's getting from Tu Trinh's sufs-test bucket
*/
void getObject(string file)
{
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {
    Aws::String FILE = Aws::Utils::StringUtils::to_string(file);
    const Aws::String bucket_name = "sufs-test";
    const Aws::String key_name = FILE;
    //const Aws::String key_name = "testfile";

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

  ifstream inFile;
  inFile.open(file);
  if(!inFile){
    cout << "Error opening/reading file." << endl;
    return;
  }

  cout << "File contents: " << endl;
  string read;
  while(getline(inFile, read)){
    cout << read;
  }
  cout << endl;
  inFile.close();
}
