#pragma once

#include "std.hpp"
#include "board.hpp"

struct State {
    Pos player;
    int nextDigit; //angka yg HARUS diinjak berikutnya, > maxDigit artinya semua angka sudah lewat
};

inline bool operator==(const State& a, const State& b) {
    return a.player.r == b.player.r && a.player.c == b.player.c && a.nextDigit == b.nextDigit;
}

struct StateHash {
    size_t operator()(const State& s) const {
        size_t h = (size_t)s.player.r * 1315423911u;
        h ^= (size_t)s.player.c + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= (size_t)s.nextDigit + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

struct Move {
    char dir; //U D L R
    State next;
    int cost; //total cost tile yg dilewati di gerakan ini (exclude tile awal)
};

struct Solution {
    bool found;
    string moves; //e.g. "RULUDRUR"
    int cost;
    vector<State> trace; //state setelah tiap move, trace[0] = state awal
    int iterations; //jumlah node yang di-pop dari frontier
    long long execMs; //waktu eksekusi solver, exclude I/O
};

State InitialState(const Board& b);
bool IsGoal(const Board& b, const State& s);
vector<Move> GenerateMoves(const Board& b, const State& s);