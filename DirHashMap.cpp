#include "DirHashEntry.cpp"
#include <string>
#include <functional>
#include <iostream>
using namespace std;

const int TABLE_SIZE = 128;

class DirHashMap {
	private:
		DirHashEntry** table;
		hash<string> hashfn;
	public:
		DirHashMap() {
			table = new DirHashEntry*[TABLE_SIZE];
			for (int i = 0; i < TABLE_SIZE; i++)
				table[i] = NULL;
		}

		bool get(string key, ContainerObject* output) {
			size_t keyHash = hashfn(key) % TABLE_SIZE;
			while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
				keyHash = (keyHash + 1) % TABLE_SIZE;
			if (table[keyHash] == NULL)
				return false;
			else {
				*output = table[keyHash]->getValue();
				return true;
			}
		}

		bool put(string key, Directory value) {
			size_t keyHash = hashfn(key) % TABLE_SIZE;
			while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
				keyHash = (keyHash + 1) % TABLE_SIZE;
			if (table[keyHash] != NULL){
				return false;
			}
			table[keyHash] = new DirHashEntry(key, value);
			return true;
		}
	  
		~DirHashMap() {
			for (int i = 0; i < TABLE_SIZE; i++)
				if (table[i] != NULL)
					delete table[i];
			delete[] table;
		}
};
