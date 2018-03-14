#include <string>
using namespace std;

class IPHashEntry {
	private:
		  string key;
		  vector<string> value;
	public:
		IPHashEntry(string key, vector<string> value) {
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