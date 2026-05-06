#pragma once

#include "std.hpp"

//pad string ke kanan sampai panjang width (tambah spasi di kiri)
string PadLeft(const string& s, int width);

//konversi int ke string lalu pad kiri
string PadInt(int v, int width);

//cari lebar maksimum representasi int dari grid 2D
int MaxIntWidth(const vector<vector<int>>& grid);
