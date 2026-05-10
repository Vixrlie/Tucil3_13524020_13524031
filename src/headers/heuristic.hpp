#pragma once

#include "std.hpp"
#include "state.hpp"

//heuristik untuk GBFS dan A*. Semua admissible (lower bound jarak Manhattan).
//heur: 1 = Manhattan player -> goal
//      2 = Manhattan player -> target berikutnya (digit yg belum diinjak / goal)
//      3 = penjumlahan Manhattan via rantai waypoint tersisa
int Heuristic(const Board& b, const State& s, int heur);
