#include <string>
#include <vector>
#include <iterator>
#include <stdio.h>

using namespace std;

class ContainerObject
{
  public:
    string name;
    string path;    

};

class File : public ContainerObject
{
  public:  
    float size;
    vector<string> blocks;
   
};

class Directory : public ContainerObject
{
  public:
    vector<File> files;
    vector<Directory> directories;

};
 
