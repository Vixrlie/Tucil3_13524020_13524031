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

    try {
        Board b = ReadBoard(path);
        PrintBoard(b, cout);

        // ask which algorithm to run
        cout << ">> Algorithm (gbfs/ucs): " << endl;
        cout << "   ";
        string alg;
        if (!getline(cin, alg)) return 0;
        size_t sa = alg.find_first_not_of(" \t");
        size_t ea = alg.find_last_not_of(" \t\r\n");
        if (sa == string::npos) return 0;
        alg = alg.substr(sa, ea - sa + 1);

        if (alg == "gbfs") {
            Solution sol = SolveGBFS(b);
            if (!sol.found) {
                cout << "Solution not found" << endl;
            } else {
                cout << "Solution moves: " << sol.moves << endl;
                cout << "Solution cost: " << sol.cost << endl;
            }
            cout << "Iterations: " << sol.iterations << endl;
            cout << "Execution time: " << sol.execMs << " ms" << endl;
        } else if (alg == "ucs") {
            // isi ucs
        } else {
            cout << "Algorithm not supported by this runner." << endl;
        }

    } catch (const exception& ex) {
        cerr << ex.what() << endl;
        return 1;
    }
    return 0;
}