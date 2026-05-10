#pragma once

#include "std.hpp"
#include "state.hpp"

//heur: 1 = Manhattan ke goal, 2 = Manhattan ke target berikut (digit/goal),
//      3 = jumlah Manhattan via rantai waypoint tersisa
Solution SolveAStar(const Board& b, int heur = 1);
