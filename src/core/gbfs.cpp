#include "headers/main.hpp"
#include "headers/gbfs.hpp"

struct Node {
	State s;
	int g;
	int parent; // index ke pool, -1 kalau root
	char dir; // '\0' kalau root
};

static int Heuristic(const Board& b, const State& s) {
	Pos target;
	if (s.nextDigit <= b.maxDigit) target = b.digits[s.nextDigit];
	else target = b.goal;
	return abs(s.player.r - target.r) + abs(s.player.c - target.c);
}

Solution SolveGBFS(const Board& b) {
	auto t0 = chrono::steady_clock::now();

	Solution sol;
	sol.found = false;
	sol.cost = 0;
	sol.iterations = 0;

	State init = InitialState(b);

	vector<Node> pool;
	pool.push_back({init, 0, -1, '\0'});

	// pq: (h, nodeIdx) min-heap
	priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
	pq.push({Heuristic(b, init), 0});

	unordered_set<State, StateHash> closed; // visited set

	int goalIdx = -1;

	while (!pq.empty()) {
		auto [h, idx] = pq.top(); pq.pop();
		sol.iterations++;

		const State cur = pool[idx].s; // copy, krn pool bisa realokasi

		if (closed.find(cur) != closed.end()) continue;
		closed.insert(cur);

		if (IsGoal(b, cur)) {
			goalIdx = idx;
			break;
		}

		for (const Move& mv : GenerateMoves(b, cur)) {
			State nxt = mv.next;
			if (closed.find(nxt) != closed.end()) continue;
			int ng = pool[idx].g + mv.cost;
			pool.push_back({nxt, ng, idx, mv.dir});
			int newIdx = (int)pool.size() - 1;
			int nh = Heuristic(b, nxt);
			pq.push({nh, newIdx});
		}
	}

	auto t1 = chrono::steady_clock::now();
	sol.execMs = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

	if (goalIdx < 0) return sol;

	// rekonstruksi jalur
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

