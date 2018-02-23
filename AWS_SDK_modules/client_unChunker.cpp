#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include <stdlib.h>
using namespace std;

//With thanks to The Coder's Lexicon

// Simply gets the file size of file.
int getFileSize(ifstream *file) {
    file->seekg(0,ios::end);
    int filesize = file->tellg();
    file->seekg(ios::beg);
    return filesize;
}

void joinFile(string chunkName, string fileOutput) {
    string fileName;
    // Create our output file
    ofstream outputfile;
    outputfile.open(fileOutput.c_str(), ios::out | ios::binary);
    // If successful, loop through chunks matching chunkName
    if (outputfile.is_open()) {
        bool filefound = true;
        int counter = 1;
        int fileSize = 0;
        while (filefound) {
            filefound = false;
            // Build the filename
            fileName.clear();
            fileName.append(chunkName);
            fileName.append(".");
 
            string intBuf = to_string(counter); 
            fileName.append(intBuf);
 
            // Open chunk to read
            ifstream fileInput;
            fileInput.open(fileName.c_str(), ios::in | ios::binary);
 
            // If chunk opened successfully, read it and write it to 
            // output file.
            if (fileInput.is_open()) {
                filefound = true;
                fileSize = getFileSize(&fileInput);
                char *inputBuffer = new char[fileSize];
 
                fileInput.read(inputBuffer,fileSize);
                outputfile.write(inputBuffer,fileSize);
                delete[] inputBuffer;
 
                fileInput.close();
            }
            counter++;
        }
        outputfile.close();
 
        cout << "File assembly complete!" << endl;
    }
    else { cout << "Error: Unable to open file for output." << endl; }
 
}
 
int main(){
  string chunkName;
  string output;
  cout << "Input chunk name without sequence no:";
  cin >> chunkName;
  cout <<"Input output file name:";
  cin >> output;
  joinFile(chunkName, output);
}
