#include <string>
using namespace std;

class NodeHashEntry {
	private:
		  string key;
		  vector<string> value;
	public:
		NodeHashEntry(string key, vector<string> value) {
			this->key = key;
			this->value = value;
		}

		string getKey() {
			return key;
		}

		vector<string> getValue() {
			return value;
		}
};
