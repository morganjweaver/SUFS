#include "NodeHashEntry.h"
#include <string>
using namespace std;

const int TABLE_SIZE = 128;

class NodeHashMap {
private:
      NodeHashEntry** table;
public:
      NodeHashMap() {
            table = new NodeHashEntry*[TABLE_SIZE];
            for (int i = 0; i < TABLE_SIZE; i++)
                table[i] = NULL;
      }

      bool get(string key, string* output) {
            int keyHash = static_cast<int>(hash(key));
            while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
                keyHash = (keyHash + 1) % TABLE_SIZE;
            if (table[keyHash] == NULL)
                return false;
            else
				*output = table[keyHash]->getValue();
                return true;
      }

      bool put(string key, string value) {
            int keyHash = static_cast<int>(hash(key));
            while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
                keyHash = (keyHash + 1) % TABLE_SIZE;
            if (table[keyHash] != NULL)
				return false;
            table[keyHash] = new NodeHashEntry(key, value);
			return true;
      }
	  
      ~NodeHashMap() {
            for (int i = 0; i < TABLE_SIZE; i++)
                  if (table[i] != NULL)
                        delete table[i];
            delete[] table;
      }
};