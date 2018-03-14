#include "ChunkHashEntry.cpp"
#include <string>
#include <functional>
#include <iostream>
using namespace std;

class ChunkHashMap {
	private:
		//ChunkHashEntry** table;
		hash<string> hashfn;
	public:
		ChunkHashEntry** table;
		ChunkHashMap() {
			table = new ChunkHashEntry*[TABLE_SIZE];
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

		bool put(string key, string value) {
			vector<string> holdValues;
			size_t keyHash = hashfn(key) % TABLE_SIZE;
			while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
				keyHash = (keyHash + 1) % TABLE_SIZE;
			if (table[keyHash] != NULL){
				holdValues = table[keyHash]->getValue();
				delete table[keyHash];
				for(int i = 0; i < holdValues.size(); i++){
					if (holdValues[i] == value)
						holdValues.erase(holdValues.begin()+i);
				}
				holdValues.push_back(value);
				table[keyHash] = new ChunkHashEntry(key, holdValues);
				return true;
			}
			holdValues.push_back(value);
			table[keyHash] = new ChunkHashEntry(key, holdValues);
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
		  
		~ChunkHashMap() {
			for (int i = 0; i < TABLE_SIZE; i++)
			  if (table[i] != NULL)
				delete table[i];
			delete[] table;
		}
};

