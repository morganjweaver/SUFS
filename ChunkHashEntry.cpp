#include <string>
using namespace std;

class ChunkHashEntry {
	private:
		  string key;
		  //vector<string> value;
	public:
		vector<string> value;
		ChunkHashEntry(string key, vector<string> value) {
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