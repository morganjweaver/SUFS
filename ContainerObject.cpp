#include <string>
#include <vector>
#include <iterator>
#include <stdio.h>
#include "Block.h"

using namespace std;

class File
{
  public:
	string fileName;
	string filePath;
    float size;
    vector<Block> blocks;
};

class Directory
{
  public:
	string dirName;
	string dirPath;
    vector<File> files;
    vector<Directory> directories;
};
 
class ContainerObject: public File, public Directory
{
};
