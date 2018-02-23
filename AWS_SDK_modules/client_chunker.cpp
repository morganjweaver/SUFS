#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include <stdlib.h>
using namespace std;

// Chunks a file by breaking it up into chunks of "chunkSize" bytes.
// With thanks to The Coder's Lexicon

const long chunkSize = 67108864; //64 Mb in bytes

void chunkFile(string fullFilePath, string chunkName) {

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
int main (int argc, char* argv[]){
  string filename;
  string chunkname; 
  cout << "Please input filename:";
  cin >> filename;
  cout << "Please input chunk base name:";
  cin >> chunkname;
  chunkFile(filename.c_str(), chunkname.c_str());
}

