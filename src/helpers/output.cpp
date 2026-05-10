#include "../headers/main.hpp"

//render papan dgn aktor di posisi state s. Tile angka yg sudah dilewati ditampilkan jadi '*'
static string RenderBoard(const Board& b, const State& s) {
    string r;
    for (int i = 0; i < b.n; i++) {
        for (int j = 0; j < b.m; j++) {
            if (i == s.player.r && j == s.player.c) { r += 'Z'; continue; }
            char ch = b.grid[i][j];
            if (ch == 'Z') { r += '*'; continue; } //posisi awal jadi tile biasa
            if (ch >= '0' && ch <= '9') {
                int d = ch - '0';
                if (d < s.nextDigit) { r += '*'; continue; } //sudah dilewati
            }
            r += ch;
        }
        r += '\n';
    }
    return r;
}

void PrintSolution(const Board& b, const Solution& sol, ostream& out) {
    if (!sol.found) {
        out << "No solution found." << "\n";
        out << "Iterations : " << sol.iterations << "\n";
        out << "Exec time  : " << FormatExecTime(sol.execUs) << "\n";
        return;
    }

    out << "Solution   : " << sol.moves << "\n";
    out << "Total cost : " << sol.cost << "\n";
    out << "Iterations : " << sol.iterations << "\n";
    out << "Exec time  : " << FormatExecTime(sol.execUs) << "\n";
    out << "\n";

    out << "Initial" << "\n";
    out << RenderBoard(b, sol.trace[0]);
    for (size_t i = 1; i < sol.trace.size(); i++) {
        out << "\n" << "Step " << i << " : " << sol.moves[i - 1] << "\n";
        out << RenderBoard(b, sol.trace[i]);
    }
}
