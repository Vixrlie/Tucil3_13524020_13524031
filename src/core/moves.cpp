#include "headers/main.hpp"

State InitialState(const Board& b) {
    State s;
    s.player = b.start;
    s.nextDigit = 0;
    if (b.maxDigit < 0) s.nextDigit = 1; //tidak ada angka, anggap selesai
    return s;
}

bool IsGoal(const Board& b, const State& s) {
    if (s.player.r != b.goal.r || s.player.c != b.goal.c) return false;
    if (b.maxDigit < 0) return true;
    return s.nextDigit > b.maxDigit;
}

static bool InBounds(const Board& b, int r, int c) {
    return r >= 0 && r < b.n && c >= 0 && c < b.m;
}

//simulasi sliding ke arah (dr, dc), return Move valid kalau bisa berhenti
static bool SlideOne(const Board& b, const State& s, int dr, int dc, char dirChar, Move& out) {
    int r = s.player.r;
    int c = s.player.c;
    int totalCost = 0;
    int curDigit = s.nextDigit;

    while (true) {
        int nr = r + dr;
        int nc = c + dc;

        //lolos pinggir tanpa rintangan = game over
        if (!InBounds(b, nr, nc)) return false;

        char tile = b.grid[nr][nc];

        //kena rintangan = berhenti di posisi sekarang
        if (tile == 'X') {
            if (r == s.player.r && c == s.player.c) return false; //ga gerak sama sekali
            out.dir = dirChar;
            out.next.player = {r, c};
            out.next.nextDigit = curDigit;
            out.cost = totalCost;
            return true;
        }

        //masuk ke tile (nr, nc)
        if (tile == 'L') return false; //lava = game over
        if (tile >= '0' && tile <= '9') {
            int d = tile - '0';
            if (d == curDigit) {
                curDigit++;
            } else if (d > curDigit) {
                return false; //angka masa depan = game over
            }
            // d < curDigit: sudah lewat, dianggap tile normal
        }

        totalCost += b.cost[nr][nc];
        r = nr;
        c = nc;
    }
}

vector<Move> GenerateMoves(const Board& b, const State& s) {
    vector<Move> moves;
    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};
    static const char dch[4] = {'U', 'D', 'L', 'R'};
    for (int i = 0; i < 4; i++) {
        Move m;
        if (SlideOne(b, s, dr[i], dc[i], dch[i], m)) moves.push_back(m);
    }
    return moves;
}