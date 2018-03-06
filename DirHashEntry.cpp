#include "ContainerObject.cpp"
#include <string>

class DirHashEntry {
private:
      int key;
      containerObject value;
	  
public:
      DirHashEntry(string key, ContainerObject value) {
            this->key = key;
            this->value = value;
      }

      int getKey() {
            return key;
      }

      ContainerObject getValue() {
            return value;
      }
};