#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
using namespace std;

void print(string msg, string end = "\n") {
    cout << msg + end;
}

vector<string> split(const string& str, char delimiter=' ') {
    vector<string> internal;
    stringstream ss(str); // Turn the string into a stream.
    string tok;
    string placeholder;
    while(getline(ss, tok, delimiter)) {
        if (tok.front() == '"') {
            getline(ss, placeholder, '"');
            tok += placeholder+'"';
        }
        internal.push_back(tok);
    }
    return internal;
}

int main() {
    string s = to_string(stoi("1"));
    if (s.length() > 0) {
        print("hi");
    }
    print(to_string(s.length() + 1));
    print(s);
    cout << stod("hi");
    print(to_string(stod("hi")));
}

