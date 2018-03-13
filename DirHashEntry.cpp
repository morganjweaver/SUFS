#include "ContainerObject.cpp"
#include <string>
using namespace std;

class DirHashEntry {
	private:
		string key;
		ContainerObject value;
	public:
		DirHashEntry(string key, ContainerObject value) {
			this->key = key;
			this->value = value;
		}

		string getKey() {
			return key;
		}

		ContainerObject getValue() {
			return value;
		}
};