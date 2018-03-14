#include "IPHashEntry.cpp"
#include <string>
#include <functional>
#include <iostream>
using namespace std;

class IPHashMap {
	private:
		IPHashEntry** table;
		hash<string> hashfn;
	public:
		IPHashMap() {
			table = new IPHashEntry*[TABLE_SIZE];
			for (int i = 0; i < TABLE_SIZE; i++)
				table[i] = NULL;
		}

		bool get(string key, vector<string>& output) {
			size_t keyHash = hashfn(key) % TABLE_SIZE;
			while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
				keyHash = (keyHash + 1) % TABLE_SIZE;
			if (table[keyHash] == NULL)
				return false;
			else {
				output = table[keyHash]->getValue();
				return true;
			}
		}

		bool put(string key, vector<string> value) {
			size_t keyHash = hashfn(key) % TABLE_SIZE;
			while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
				keyHash = (keyHash + 1) % TABLE_SIZE;
			if (table[keyHash] != NULL){
				table[keyHash] = NULL;
				delete table[keyHash];
			}
			table[keyHash] = new IPHashEntry(key, value);
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
		  
		~IPHashMap() {
			for (int i = 0; i < TABLE_SIZE; i++)
			  if (table[i] != NULL)
				delete table[i];
			delete[] table;
		}
};
