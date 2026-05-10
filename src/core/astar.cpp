#include "headers/main.hpp"

struct Node {
    State s;
    int g;
    int parent; //index ke pool, -1 kalau root
    char dir; //\'\0\' kalau root
};

struct PQItem {
    int f;
    int negG;
    long long id;
    int idx;
};

struct PQComp {
    bool operator()(const PQItem& a, const PQItem& b) const {
        if (a.f != b.f) return a.f > b.f;
        if (a.negG != b.negG) return a.negG > b.negG;
        return a.id > b.id;
    }
};

static int Heuristic(const Board& b, const State& s) {
    return abs(s.player.r - b.goal.r) + abs(s.player.c - b.goal.c);
}

Solution SolveAStar(const Board& b) {
    auto t0 = chrono::steady_clock::now();

    Solution sol;
    sol.found = false;
    sol.cost = 0;
    sol.iterations = 0;

    State init = InitialState(b);

    vector<Node> pool;
    pool.push_back({init, 0, -1, '\0'});

    priority_queue<PQItem, vector<PQItem>, PQComp> pq;
    int h0 = Heuristic(b, init);
    pq.push({h0, 0, 0, 0});

    unordered_map<State, int, StateHash> bestG; //best g per state
    bestG[init] = 0;

    int goalIdx = -1;
    long long pushId = 1;

    while (!pq.empty()) {
        PQItem it = pq.top(); pq.pop();
        sol.iterations++;

        const State cur = pool[it.idx].s; //copy, krn pool bisa realokasi

        auto bg = bestG.find(cur);
        if (bg != bestG.end() && pool[it.idx].g > bg->second) continue;

        if (IsGoal(b, cur)) {
            goalIdx = it.idx;
            break;
        }

        for (const Move& mv : GenerateMoves(b, cur)) {
            int ng = pool[it.idx].g + mv.cost;
            auto it2 = bestG.find(mv.next);
            if (it2 != bestG.end() && ng >= it2->second) continue;
            bestG[mv.next] = ng;
            pool.push_back({mv.next, ng, it.idx, mv.dir});
            int nh = Heuristic(b, mv.next);
            int nf = ng + nh;
            pq.push({nf, -ng, pushId, (int)pool.size() - 1});
            pushId++;
        }
    }

    auto t1 = chrono::steady_clock::now();
    sol.execUs = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

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
