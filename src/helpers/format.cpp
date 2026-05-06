#include "../headers/main.hpp"

string PadLeft(const string& s, int width) {
    if ((int)s.size() >= width) return s;
    return string(width - s.size(), ' ') + s;
}

string PadInt(int v, int width) {
    return PadLeft(to_string(v), width);
}

int MaxIntWidth(const vector<vector<int>>& grid) {
    int w = 1;
    for (const auto& row : grid) {
        for (int v : row) {
            int len = (int)to_string(v).size();
            if (len > w) w = len;
        }
    }
    return w;
}
