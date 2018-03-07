#include <string>
using namespace std;

class NodeHashEntry {
private:
      string key;
      string value;
	  
public:
      DNodeIDHashEntry(string key, string value) {
            this->key = key;
            this->value = value;
      }

      int getKey() {
            return key;
      }

      string getValue() {
            return value;
      }
};