#include "headers/main.hpp"

//---- helper ----

static bool IsValidTile(char ch) {
    if (ch == 'X' || ch == '*' || ch == 'L' || ch == 'Z' || ch == 'O') return true;
    if (ch >= '0' && ch <= '9') return true;
    return false;
}

static void Fail(const string& msg) {
    throw runtime_error("[ERROR] " + msg + " !");
}

//---- read ----

Board ReadBoard(const string& path) {
    ifstream in(path);
    if (!in.is_open()) Fail("Unable to open file '" + path + "'");

    Board b;
    b.n = 0; b.m = 0;
    b.start = {-1, -1};
    b.goal = {-1, -1};
    b.maxDigit = -1;
    b.digits.assign(10, Pos{-1, -1});

    if (!(in >> b.n >> b.m)) Fail("Header N M is missing or unreadable");
    if (b.n <= 0 || b.m <= 0) Fail("N and M must be positive");
    in.ignore(numeric_limits<streamsize>::max(), '\n');

    b.grid.reserve(b.n);
    int zCount = 0; int oCount = 0;
    set<int> seenDigits;
    for (int i = 0; i < b.n; i++) {
        string row;
        if (!getline(in, row)) Fail("Map row " + to_string(i) + " is missing");
        if (!row.empty() && row.back() == '\r') row.pop_back(); //CRLF dari windows
        if ((int)row.size() != b.m) Fail("Map row " + to_string(i) + " length does not match M");
        for (int j = 0; j < b.m; j++) {
            char ch = row[j];
            if (!IsValidTile(ch)) Fail("Unknown tile character '" + string(1, ch) + "' at (" + to_string(i) + "," + to_string(j) + ")");
            if (ch == 'Z') { b.start = {i, j}; zCount++; }
            else if (ch == 'O') { b.goal = {i, j}; oCount++; }
            else if (ch >= '0' && ch <= '9') {
                int d = ch - '0';
                if (b.digits[d].r != -1) Fail("Digit '" + string(1, ch) + "' appears more than once");
                b.digits[d] = {i, j};
                seenDigits.insert(d);
                if (d > b.maxDigit) b.maxDigit = d;
            }
        }
        b.grid.push_back(row);
    }
    if (zCount != 1) Fail("Z must appear exactly once, found " + to_string(zCount));
    if (oCount != 1) Fail("O must appear exactly once, found " + to_string(oCount));

    if (b.maxDigit >= 0) { //cek angka kontigu 0..maxDigit
        for (int d = 0; d <= b.maxDigit; d++) {
            if (!seenDigits.count(d)) Fail("Digit sequence has a gap, missing " + to_string(d) + " while max is " + to_string(b.maxDigit));
        }
    }

    b.cost.assign(b.n, vector<int>(b.m, 0));
    for (int i = 0; i < b.n; i++) {
        for (int j = 0; j < b.m; j++) {
            if (!(in >> b.cost[i][j])) Fail("Cost value missing at (" + to_string(i) + "," + to_string(j) + ")");
        }
    }

    string extra;
    while (in >> extra) Fail("Extra token after cost grid: '" + extra + "'"); //tolak sisa

    return b;
}

//---- print ----

void PrintBoard(const Board& b, ostream& out) {
    out << "N=" << b.n << " M=" << b.m << "\n";
    out << "Start: (" << b.start.r << "," << b.start.c << ")  Goal: (" << b.goal.r << "," << b.goal.c << ")  maxDigit=" << b.maxDigit << "\n";
    out << "Grid:\n";
    for (int i = 0; i < b.n; i++) out << "  " << b.grid[i] << "\n";
    out << "Cost:\n";
    int w = MaxIntWidth(b.cost);
    for (int i = 0; i < b.n; i++) {
        out << "  ";
        for (int j = 0; j < b.m; j++) {
            out << PadInt(b.cost[i][j], w);
            if (j + 1 < b.m) out << " ";
        }
        out << "\n";
    }
}
