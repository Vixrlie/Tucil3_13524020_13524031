
#pragma once

#include "std.hpp"
#include "state.hpp"

//heur: 1=H1, 2=H2, 3=H3. Default H2.
Solution SolveGBFS(const Board& b, int heur = 2);
