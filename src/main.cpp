#include "headers/main.hpp"

int main() {
    cout << ">> Input file name : " << endl;
    cout << "   ";
    string name;
    if (!getline(cin, name)) return 1;
    size_t s = name.find_first_not_of(" \t");
    size_t e = name.find_last_not_of(" \t\r\n");
    if (s == string::npos) { cerr << "[ERROR] Empty file name !" << endl; return 1; }
    name = name.substr(s, e - s + 1);

    string path = "test/" + name;

    try {
        Board b = ReadBoard(path);
        PrintBoard(b, cout);
    } catch (const exception& ex) {
        cerr << ex.what() << endl;
        return 1;
    }
    return 0;
}
