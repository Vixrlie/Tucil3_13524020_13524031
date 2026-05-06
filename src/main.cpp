#include "headers/main.hpp"

static string Trim(const string& s) {
    size_t a = s.find_first_not_of(" \t");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == string::npos) return "";
    return s.substr(a, b - a + 1);
}

int main() {
    cout << ">> Input file name : " << endl;
    cout << "   ";
    string name;
    if (!getline(cin, name)) return 1;
    name = Trim(name);
    if (name.empty()) { cerr << "[ERROR] Empty file name !" << endl; return 1; }

    string path = "test/" + name;

    Board b;
    try {
        b = ReadBoard(path);
    } catch (const exception& ex) {
        cerr << ex.what() << endl;
        return 1;
    }

    cout << ">> Algorithm (UCS) : " << endl;
    cout << "   ";
    string algo;
    if (!getline(cin, algo)) return 1;
    algo = Trim(algo);
    for (char& ch : algo) ch = (char)toupper((unsigned char)ch);

    Solution sol;
    if (algo == "UCS") sol = SolveUCS(b);
    else { cerr << "[ERROR] Unknown algorithm '" << algo << "' !" << endl; return 1; }

    PrintSolution(b, sol, cout);
    return 0;
}
