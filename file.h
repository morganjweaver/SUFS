#ifndef FILE_H
#define FILE_H
#include <string>

struct file {
  string filename;
  string path;
  int size;
  vector <block> blocks;
};

#endif
