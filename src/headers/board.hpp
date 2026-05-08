#pragma once

#include "std.hpp"

struct Pos { int r; int c; }; //posisi tile

struct Board {
    int n; //banyak baris
    int m; //banyak kolom
    vector<string> grid; //peta n x m
    vector<vector<int>> cost; //cost tiap tile
    Pos start; //posisi Z
    Pos goal; //posisi O
    vector<Pos> digits; //digits[i] = posisi angka i, kalau ga ada r=-1
    int maxDigit; //angka tertinggi yang muncul, -1 kalau ga ada angka sama sekali
};

Board ReadBoard(const string& path); //lempar runtime_error kalau invalid
void PrintBoard(const Board& b, ostream& out);
