#include "headers/main.hpp"

static string Trim(const string& s) {
    size_t a = s.find_first_not_of(" \t");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == string::npos) return "";
    return s.substr(a, b - a + 1);
}

static int RunCli() {
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
        cout << ">> Algorithm (gbfs/ucs/a*/astar): " << endl;
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
            cout << "Execution time: " << sol.execUs << " ms" << endl;
        } else if (alg == "ucs") {
            Solution sol = SolveUCS(b);
            if (!sol.found) {
                cout << "Solution not found" << endl;
            } else {
                cout << "Solution moves: " << sol.moves << endl;
                cout << "Solution cost: " << sol.cost << endl;
            }
            cout << "Iterations: " << sol.iterations << endl;
            cout << "Execution time: " << sol.execUs << " ms" << endl;
        } else if (alg == "a*" || alg == "astar") {
            cout << ">> Heuristic (h1/h2/h3): " << endl;
            cout << "   ";
            string heur;
            if (!getline(cin, heur)) return 0;
            size_t sh = heur.find_first_not_of(" \t");
            size_t eh = heur.find_last_not_of(" \t\r\n");
            if (sh != string::npos) heur = heur.substr(sh, eh - sh + 1);
            Solution sol = SolveAStar(b);
            if (!sol.found) {
                cout << "Solution not found" << endl;
            } else {
                cout << "Solution moves: " << sol.moves << endl;
                cout << "Solution cost: " << sol.cost << endl;
            }
            cout << "Iterations: " << sol.iterations << endl;
            cout << "Execution time: " << sol.execUs << " ms" << endl;
        } else {
            cout << "Algorithm not supported by this runner." << endl;
        }

    } catch (const exception& ex) {
        cerr << ex.what() << endl;
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    //default: GUI. Pakai --cli untuk mode terminal.
    for (int i = 1; i < argc; i++) {
        string a = argv[i];
        if (a == "--cli" || a == "-c") return RunCli();
    }
    return RunGui();
}
