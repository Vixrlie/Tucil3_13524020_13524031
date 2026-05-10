#include "../headers/main.hpp"

static inline int Manh(Pos a, Pos c) {
    return abs(a.r - c.r) + abs(a.c - c.c);
}

//H1: Manhattan player -> goal. Mengabaikan kewajiban lewat digit.
static int H1(const Board& b, const State& s) {
    return Manh(s.player, b.goal);
}

//H2: Manhattan player -> target berikutnya (digit yg belum diinjak / goal).
static int H2(const Board& b, const State& s) {
    if (s.nextDigit <= b.maxDigit) return Manh(s.player, b.digits[s.nextDigit]);
    return Manh(s.player, b.goal);
}

//H3: penjumlahan Manhattan via rantai waypoint tersisa.
//    player -> nextDigit -> nextDigit+1 -> ... -> maxDigit -> goal
static int H3(const Board& b, const State& s) {
    int total = 0;
    Pos cur = s.player;
    for (int d = s.nextDigit; d <= b.maxDigit; d++) {
        total += Manh(cur, b.digits[d]);
        cur = b.digits[d];
    }
    total += Manh(cur, b.goal);
    return total;
}

int Heuristic(const Board& b, const State& s, int heur) {
    if (heur == 1) return H1(b, s);
    if (heur == 3) return H3(b, s);
    return H2(b, s); //default
}
