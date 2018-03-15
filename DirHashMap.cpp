#include "DirHashEntry.cpp"
#include <string>
#include <functional>
#include <iostream>
using namespace std;

const int TABLE_SIZE = 2000;

class DirHashMap {
	private:
		hash<string> hashfn;
	public:
		DirHashEntry** table;
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

		bool put(string key, ContainerObject value) {
			size_t keyHash = hashfn(key) % TABLE_SIZE;
			
			while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
				keyHash = (keyHash + 1) % TABLE_SIZE;
				cout << "key hashes: " << keyHash << endl;
			if (table[keyHash] != NULL){
				table[keyHash] = NULL;
				delete table[keyHash];
				table[keyHash] = new DirHashEntry(key, value);
			}
			table[keyHash] = new DirHashEntry(key, value);
			return true;
		}
		
		bool remove(string key){
			size_t keyHash = hashfn(key) % TABLE_SIZE;
			while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
				keyHash = (keyHash + 1) % TABLE_SIZE;
			if (table[keyHash] == NULL){
				return false;
			}
			table[keyHash] = NULL;
			delete table[keyHash];
			return true;
		}
	  
		~DirHashMap() {
			for (int i = 0; i < TABLE_SIZE; i++)
				if (table[i] != NULL)
					delete table[i];
			delete[] table;
		}
};
