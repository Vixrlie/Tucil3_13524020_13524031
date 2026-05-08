#include "../headers/main.hpp"

struct Node {
    State s;
    int g;
    int parent; //index ke pool, -1 kalau root
    char dir; //'\0' kalau root
};

Solution SolveUCS(const Board& b) {
    auto t0 = chrono::steady_clock::now();

    Solution sol;
    sol.found = false;
    sol.cost = 0;
    sol.iterations = 0;

    State init = InitialState(b);

    vector<Node> pool;
    pool.push_back({init, 0, -1, '\0'});

    //pq: (g, nodeIdx), min-heap
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    pq.push({0, 0});

    unordered_map<State, int, StateHash> bestG; //best g per state
    bestG[init] = 0;

    int goalIdx = -1;

    while (!pq.empty()) {
        auto [g, idx] = pq.top(); pq.pop();
        sol.iterations++;

        const State cur = pool[idx].s; //copy, krn pool bisa realokasi

        //skip kalau sudah ada jalur lebih baik ke state ini
        auto it = bestG.find(cur);
        if (it != bestG.end() && g > it->second) continue;

        if (IsGoal(b, cur)) {
            goalIdx = idx;
            break;
        }

        for (const Move& mv : GenerateMoves(b, cur)) {
            int ng = g + mv.cost;
            auto it2 = bestG.find(mv.next);
            if (it2 != bestG.end() && ng >= it2->second) continue;
            bestG[mv.next] = ng;
            pool.push_back({mv.next, ng, idx, mv.dir});
            pq.push({ng, (int)pool.size() - 1});
        }
    }

    auto t1 = chrono::steady_clock::now();
    sol.execMs = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

    if (goalIdx < 0) return sol;

    //rekonstruksi jalur
    sol.found = true;
    sol.cost = pool[goalIdx].g;
    vector<char> dirs;
    vector<State> states;
    int cur = goalIdx;
    while (cur != -1) {
        states.push_back(pool[cur].s);
        if (pool[cur].dir != '\0') dirs.push_back(pool[cur].dir);
        cur = pool[cur].parent;
    }
    reverse(dirs.begin(), dirs.end());
    reverse(states.begin(), states.end());
    sol.moves = string(dirs.begin(), dirs.end());
    sol.trace = states;
    return sol;
}
