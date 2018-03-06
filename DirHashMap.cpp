#include "DirHashEntry.h"
#include <string>
using namespace std;

const int TABLE_SIZE = 128;

class DirHashMap {
private:
      HashEntry** table;
public:
      DirHashMap() {
            table = new HashEntry*[TABLE_SIZE];
            for (int i = 0; i < TABLE_SIZE; i++)
                table[i] = NULL;
      }

      bool get(string key, ContainerObject* output) {
            int keyHash = static_cast<int>(hash(key));
            while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
                keyHash = (keyHash + 1) % TABLE_SIZE;
            if (table[keyHash] == NULL)
                return false;
            else
				*output = table[keyHash]->getValue();
                return true;
      }

      bool put(string key, ContainerObject value) {
            int keyHash = static_cast<int>(hash(key));
            while (table[keyHash] != NULL && table[keyHash]->getKey() != key)
                keyHash = (keyHash + 1) % TABLE_SIZE;
            if (table[keyHash] != NULL)
				return false;
            table[keyHash] = new HashEntry(key, value);
			return true;
      }
	  
      ~DirHashMap() {
            for (int i = 0; i < TABLE_SIZE; i++)
                  if (table[i] != NULL)
                        delete table[i];
            delete[] table;
      }
};