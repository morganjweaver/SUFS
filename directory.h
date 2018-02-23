#ifndef DIRECTORY_H
#define DIRECTORY_H
#include <string>
#include <vector>

struct directory {
  string dirname;
  string path;
  vector<file> files;
  vector<directory> directories;
};

#endif
