#include "../headers/main.hpp"
#include <raylib.h>
#include <dirent.h>
#include <sys/stat.h>

//warna palet
static const Color BG          = {  18,  20,  28, 255 };
static const Color PANEL       = {  28,  32,  44, 255 };
static const Color PANEL_HI    = {  40,  46,  62, 255 };
static const Color BORDER      = {  60,  68,  88, 255 };
static const Color TEXT        = { 220, 226, 240, 255 };
static const Color TEXT_DIM    = { 150, 158, 178, 255 };
static const Color ACCENT      = {  90, 160, 255, 255 };
static const Color ACCENT_HOT  = { 120, 180, 255, 255 };
static const Color OK          = {  90, 200, 130, 255 };
static const Color WARN        = { 240, 180,  80, 255 };
static const Color BAD         = { 230,  90,  90, 255 };

//warna tile
static const Color TILE_PATH   = {  50,  56,  72, 255 };
static const Color TILE_WALL   = { 220, 226, 240, 255 };
static const Color TILE_LAVA   = { 220,  80,  60, 255 };
static const Color TILE_GOAL   = {  80, 200, 140, 255 };
static const Color TILE_DIGIT  = { 240, 200,  80, 255 };
static const Color PLAYER      = {  80, 150, 240, 255 };

static int gW = 1100;
static int gH = 700;

//pecah teks jadi baris2 yg muat di width pixel tertentu (word-wrap, fallback char-wrap)
static vector<string> WrapText(const string& text, int fontSize, int maxWidth) {
    vector<string> lines;
    string line;
    string word;
    auto flushWord = [&](bool addSpace) {
        if (word.empty()) return;
        string cand = line.empty() ? word : line + " " + word;
        if (MeasureText(cand.c_str(), fontSize) <= maxWidth) {
            line = cand;
        } else {
            if (!line.empty()) { lines.push_back(line); line.clear(); }
            //word sendiri mungkin lebih lebar dari maxWidth, hard-wrap per char
            if (MeasureText(word.c_str(), fontSize) > maxWidth) {
                string chunk;
                for (char ch : word) {
                    string trial = chunk + ch;
                    if (MeasureText(trial.c_str(), fontSize) > maxWidth && !chunk.empty()) {
                        lines.push_back(chunk);
                        chunk.clear();
                    }
                    chunk += ch;
                }
                line = chunk;
            } else {
                line = word;
            }
        }
        word.clear();
        (void)addSpace;
    };
    for (char ch : text) {
        if (ch == '\n') {
            flushWord(false);
            lines.push_back(line);
            line.clear();
        } else if (ch == ' ' || ch == '\t') {
            flushWord(true);
        } else {
            word += ch;
        }
    }
    flushWord(false);
    if (!line.empty()) lines.push_back(line);
    return lines;
}

//list file di folder test/
static vector<string> ListTestFiles() {
    vector<string> out;
    DIR* d = opendir("test");
    if (!d) return out;
    struct dirent* ent;
    while ((ent = readdir(d)) != nullptr) {
        string n = ent->d_name;
        if (n.size() > 4 && n.substr(n.size() - 4) == ".txt") out.push_back(n);
    }
    closedir(d);
    sort(out.begin(), out.end());
    return out;
}

//gambar tile sesuai karakter board (state aware)
static void DrawTile(int x, int y, int sz, char ch, int nextDigit) {
    Color fill = TILE_PATH;
    bool drawText = false;
    char label = ' ';
    Color labelColor = TEXT;

    if (ch == 'X') fill = TILE_WALL;
    else if (ch == 'L') { fill = TILE_LAVA; drawText = true; label = 'L'; labelColor = TILE_WALL; }
    else if (ch == 'O') { fill = TILE_GOAL; drawText = true; label = 'O'; labelColor = BG; }
    else if (ch >= '0' && ch <= '9') {
        int d = ch - '0';
        if (d < nextDigit) fill = TILE_PATH; //sudah dilewati, tampil biasa
        else { fill = TILE_DIGIT; drawText = true; label = ch; labelColor = BG; }
    }

    DrawRectangle(x, y, sz - 1, sz - 1, fill);
    if (drawText) {
        char s[2] = { label, 0 };
        int fs = sz * 6 / 10;
        int tw = MeasureText(s, fs);
        DrawText(s, x + (sz - tw) / 2, y + (sz - fs) / 2, fs, labelColor);
    }
}

//gambar player di posisi (r,c)
static void DrawPlayer(int gridX, int gridY, int sz, int r, int c) {
    int cx = gridX + c * sz + sz / 2;
    int cy = gridY + r * sz + sz / 2;
    DrawCircle(cx, cy, sz * 0.35f, PLAYER);
    DrawCircleLines(cx, cy, sz * 0.35f, ACCENT_HOT);
}

//tombol simple, return true kalau diklik
static bool Button(Rectangle r, const char* label, bool enabled = true) {
    Vector2 mp = GetMousePosition();
    bool hover = enabled && CheckCollisionPointRec(mp, r);
    bool clicked = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    Color bg = enabled ? (hover ? PANEL_HI : PANEL) : PANEL;
    Color border = enabled ? (hover ? ACCENT : BORDER) : BORDER;
    Color tc = enabled ? TEXT : TEXT_DIM;
    DrawRectangleRec(r, bg);
    DrawRectangleLinesEx(r, 1, border);
    int fs = 16;
    int tw = MeasureText(label, fs);
    DrawText(label, (int)(r.x + (r.width - tw) / 2), (int)(r.y + (r.height - fs) / 2), fs, tc);
    return clicked;
}

//dropdown sederhana, opsi dipilih dengan klik. Return idx terpilih.
struct Dropdown {
    bool open = false;
    int idx = 0;
};
static int DrawDropdown(Rectangle r, Dropdown& dd, const vector<string>& items, const char* label) {
    DrawText(label, (int)r.x, (int)r.y - 18, 14, TEXT_DIM);
    Vector2 mp = GetMousePosition();
    bool hover = CheckCollisionPointRec(mp, r);
    DrawRectangleRec(r, hover ? PANEL_HI : PANEL);
    DrawRectangleLinesEx(r, 1, hover ? ACCENT : BORDER);
    string cur = items.empty() ? string("-") : items[dd.idx];
    DrawText(cur.c_str(), (int)r.x + 10, (int)r.y + 8, 16, TEXT);
    DrawText("v", (int)(r.x + r.width - 18), (int)r.y + 8, 16, TEXT_DIM);
    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) dd.open = !dd.open;
    return dd.idx;
}
//panel popup dropdown digambar belakangan supaya nutupin elemen lain
static void DrawDropdownPopup(Rectangle r, Dropdown& dd, const vector<string>& items) {
    if (!dd.open) return;
    Vector2 mp = GetMousePosition();
    Rectangle pop = { r.x, r.y + r.height, r.width, (float)(items.size() * 30) };
    DrawRectangleRec(pop, PANEL);
    DrawRectangleLinesEx(pop, 1, BORDER);
    for (size_t i = 0; i < items.size(); i++) {
        Rectangle row = { pop.x, pop.y + i * 30, pop.width, 30 };
        bool h = CheckCollisionPointRec(mp, row);
        if (h) DrawRectangleRec(row, PANEL_HI);
        DrawText(items[i].c_str(), (int)row.x + 10, (int)row.y + 7, 16, TEXT);
        if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            dd.idx = (int)i;
            dd.open = false;
        }
    }
    //klik di luar popup tutup dropdown
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(mp, pop) && !CheckCollisionPointRec(mp, r)) {
        dd.open = false;
    }
}

//slider horizontal float [0..1]
static float Slider(Rectangle r, float v, bool& dragging) {
    Vector2 mp = GetMousePosition();
    bool hover = CheckCollisionPointRec(mp, r);
    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) dragging = true;
    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) dragging = false;
    if (dragging) {
        v = (mp.x - r.x) / r.width;
        if (v < 0) v = 0;
        if (v > 1) v = 1;
    }
    DrawRectangleRec(r, PANEL);
    DrawRectangleLinesEx(r, 1, BORDER);
    Rectangle fill = { r.x, r.y, r.width * v, r.height };
    DrawRectangleRec(fill, ACCENT);
    return v;
}

//tulis solusi ke file txt. Return true kalau sukses.
static bool SaveSolutionTxt(const string& path, const Board& b, const Solution& sol,
                            const string& algo, const string& heur) {
    ofstream of(path);
    if (!of) return false;
    of << "Algorithm   : " << algo << "\n";
    of << "Heuristic   : " << heur << "\n";
    of << "Solution    : " << sol.moves << "\n";
    of << "Total cost  : " << sol.cost << "\n";
    of << "Iterations  : " << sol.iterations << "\n";
    of << "Exec time   : " << FormatExecTime(sol.execUs) << "\n\n";
    PrintSolution(b, sol, of);
    return true;
}

int RunGui() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(gW, gH, "Ice Sliding Puzzle Solver");
    SetTargetFPS(60);

    vector<string> files = ListTestFiles();
    Dropdown ddFile, ddAlgo, ddHeur;
    vector<string> algos = { "UCS", "GBFS", "A*" };
    vector<string> heurs = { "H1", "H2", "H3" };

    bool boardLoaded = false;
    Board board;
    string loadError;

    bool solved = false;
    Solution sol;
    string solveNotice; //pesan kalau pakai stub

    int curStep = 0;
    bool playing = false;
    float playSpeed = 0.5f; //0..1, mapped ke frame interval
    float playAccum = 0.0f;

    bool sliderDrag = false;
    string saveStatus;

    while (!WindowShouldClose()) {
        gW = GetScreenWidth();
        gH = GetScreenHeight();

        //input keyboard playback
        if (solved) {
            if (IsKeyPressed(KEY_RIGHT)) {
                if (curStep < (int)sol.trace.size() - 1) curStep++;
                playing = false;
            }
            if (IsKeyPressed(KEY_LEFT)) {
                if (curStep > 0) curStep--;
                playing = false;
            }
            if (IsKeyPressed(KEY_SPACE)) playing = !playing;
            if (IsKeyPressed(KEY_HOME)) { curStep = 0; playing = false; }
            if (IsKeyPressed(KEY_END))  { curStep = (int)sol.trace.size() - 1; playing = false; }
        }

        //playback otomatis
        if (playing && solved) {
            //speed: 0 -> 1.0s/step, 1 -> 0.05s/step
            float interval = 1.0f - playSpeed * 0.95f;
            playAccum += GetFrameTime();
            if (playAccum >= interval) {
                playAccum = 0;
                if (curStep < (int)sol.trace.size() - 1) curStep++;
                else playing = false;
            }
        }

        BeginDrawing();
        ClearBackground(BG);

        //sidebar
        const int SIDE_W = 290;
        DrawRectangle(0, 0, SIDE_W, gH, PANEL);
        DrawLine(SIDE_W, 0, SIDE_W, gH, BORDER);

        DrawText("Ice Sliding Puzzle", 20, 18, 20, TEXT);
        DrawText("Solver", 20, 42, 14, TEXT_DIM);

        int y = 90;
        Rectangle rFile = { 20, (float)y, SIDE_W - 40, 32 };
        DrawDropdown(rFile, ddFile, files, "Input file");
        y += 60;

        Rectangle rAlgo = { 20, (float)y, SIDE_W - 40, 32 };
        DrawDropdown(rAlgo, ddAlgo, algos, "Algorithm");
        y += 60;

        Rectangle rHeur = { 20, (float)y, SIDE_W - 40, 32 };
        bool heurEnabled = (ddAlgo.idx != 0); //UCS ga butuh heuristik
        if (heurEnabled) {
            DrawDropdown(rHeur, ddHeur, heurs, "Heuristic");
        } else {
            //placeholder disabled, hanya satu label, tanpa pemanggilan DrawDropdown supaya gak dobel
            DrawText("Heuristic", (int)rHeur.x, (int)rHeur.y - 18, 14, TEXT_DIM);
            DrawRectangleRec(rHeur, PANEL);
            DrawRectangleLinesEx(rHeur, 1, BORDER);
            DrawText("(unused for UCS)", (int)rHeur.x + 10, (int)rHeur.y + 8, 16, TEXT_DIM);
            ddHeur.open = false; //pastikan popup nutup kalau user pindah ke UCS saat dropdown terbuka
        }
        y += 60;

        Rectangle rSolve = { 20, (float)y, SIDE_W - 40, 40 };
        bool canSolve = !files.empty();
        if (Button(rSolve, "Solve", canSolve)) {
            //load board
            try {
                board = ReadBoard("test/" + files[ddFile.idx]);
                boardLoaded = true;
                loadError.clear();
            } catch (const exception& ex) {
                loadError = ex.what();
                boardLoaded = false;
            }
            if (boardLoaded) {
                //semua algo sementara fallback ke UCS (stub)
                if (ddAlgo.idx == 0) sol = SolveUCS(board);
                else if (ddAlgo.idx == 1) sol = SolveGBFS(board);
                else if (ddAlgo.idx == 2) sol = SolveAStar(board);
                solved = true;
                curStep = 0;
                playing = false;
                saveStatus.clear();
            }
        }
        y += 60;

        //info panel
        if (!loadError.empty()) {
            DrawText("Load error:", 20, y, 14, BAD);
            y += 20;
            //scrollable box: wrap teks ke lebar sidebar - padding - scrollbar
            const int errBoxW = SIDE_W - 40;
            const int errBoxMaxH = 90;
            const int padX = 8;
            const int padY = 6;
            const int scrollbarW = 6;
            const int wrapWidth = errBoxW - padX * 2 - scrollbarW;
            const int fs = 12;
            const int lineH = 14;
            vector<string> wrapped = WrapText(loadError, fs, wrapWidth);
            int contentH = (int)wrapped.size() * lineH;
            int boxH = min(contentH + padY * 2, errBoxMaxH);
            Rectangle errBox = { 20, (float)y, (float)errBoxW, (float)boxH };

            //scroll wheel saat hover
            static float errScroll = 0.0f;
            int maxScroll = max(0, contentH + padY * 2 - boxH);
            Vector2 mp = GetMousePosition();
            if (CheckCollisionPointRec(mp, errBox)) {
                errScroll -= GetMouseWheelMove() * 20.0f;
            }
            if (errScroll < 0) errScroll = 0;
            if (errScroll > maxScroll) errScroll = (float)maxScroll;

            DrawRectangleRec(errBox, PANEL);
            DrawRectangleLinesEx(errBox, 1, BAD);

            BeginScissorMode((int)errBox.x, (int)errBox.y, (int)errBox.width, (int)errBox.height);
            int textX = (int)errBox.x + padX;
            int textY = (int)errBox.y + padY - (int)errScroll;
            for (const string& ln : wrapped) {
                DrawText(ln.c_str(), textX, textY, fs, TEXT_DIM);
                textY += lineH;
            }
            EndScissorMode();

            //scrollbar
            if (maxScroll > 0) {
                float barX = errBox.x + errBox.width - scrollbarW - 2;
                float trackH = errBox.height - 4;
                float thumbH = trackH * (float)boxH / (float)(contentH + padY * 2);
                if (thumbH < 16) thumbH = 16;
                float thumbY = errBox.y + 2 + (trackH - thumbH) * (errScroll / (float)maxScroll);
                DrawRectangle((int)barX, (int)errBox.y + 2, scrollbarW, (int)trackH, BORDER);
                DrawRectangle((int)barX, (int)thumbY, scrollbarW, (int)thumbH, ACCENT);
            }
            y += boxH + 12;
        }

        if (solved) {
            DrawText("Result", 20, y, 14, TEXT_DIM); y += 20;
            char buf[256];
            if (sol.found) {
                snprintf(buf, sizeof(buf), "Status     : FOUND");
                DrawText(buf, 20, y, 14, OK); y += 20;
                snprintf(buf, sizeof(buf), "Moves      : %s", sol.moves.c_str());
                DrawText(buf, 20, y, 14, TEXT); y += 20;
                snprintf(buf, sizeof(buf), "Total cost : %d", sol.cost);
                DrawText(buf, 20, y, 14, TEXT); y += 20;
            } else {
                DrawText("Status     : NOT FOUND", 20, y, 14, BAD); y += 20;
            }
            snprintf(buf, sizeof(buf), "Iterations : %d", sol.iterations);
            DrawText(buf, 20, y, 14, TEXT); y += 20;
            snprintf(buf, sizeof(buf), "Exec time  : %s", FormatExecTime(sol.execUs).c_str());
            DrawText(buf, 20, y, 14, TEXT); y += 28;

            if (!solveNotice.empty()) {
                DrawText("Note:", 20, y, 12, WARN); y += 16;
                //wrap manual sederhana
                string s = solveNotice;
                size_t pos = 0;
                while (pos < s.size()) {
                    size_t take = min((size_t)34, s.size() - pos);
                    DrawText(s.substr(pos, take).c_str(), 20, y, 12, TEXT_DIM);
                    pos += take;
                    y += 14;
                }
                y += 8;
            }

            if (sol.found) {
                //playback controls
                DrawText("Playback", 20, y, 14, TEXT_DIM); y += 22;
                snprintf(buf, sizeof(buf), "Step %d / %zu", curStep, sol.trace.size() - 1);
                DrawText(buf, 20, y, 14, TEXT); y += 22;

                Rectangle rPrev  = { 20,  (float)y, 60, 30 };
                Rectangle rPlay  = { 90,  (float)y, 80, 30 };
                Rectangle rNext  = { 180, (float)y, 60, 30 };
                if (Button(rPrev, "<")) { if (curStep > 0) curStep--; playing = false; }
                if (Button(rPlay, playing ? "Pause" : "Play")) playing = !playing;
                if (Button(rNext, ">")) { if (curStep < (int)sol.trace.size() - 1) curStep++; playing = false; }
                y += 40;

                //slider step
                Rectangle rStep = { 20, (float)y, SIDE_W - 40, 16 };
                float sv = sol.trace.size() > 1 ? (float)curStep / (float)(sol.trace.size() - 1) : 0.0f;
                float nv = Slider(rStep, sv, sliderDrag);
                if (sliderDrag) {
                    curStep = (int)(nv * (sol.trace.size() - 1) + 0.5f);
                    playing = false;
                }
                y += 24;

                //slider speed
                DrawText("Speed", 20, y, 12, TEXT_DIM); y += 16;
                Rectangle rSpd = { 20, (float)y, SIDE_W - 40, 16 };
                static bool spdDrag = false;
                playSpeed = Slider(rSpd, playSpeed, spdDrag);
                y += 28;

                //save
                Rectangle rSave = { 20, (float)y, SIDE_W - 40, 32 };
                if (Button(rSave, "Save solution to .txt")) {
                    string outPath = "test/solution_" + files[ddFile.idx];
                    bool ok = SaveSolutionTxt(outPath, board, sol,
                                              algos[ddAlgo.idx],
                                              heurEnabled ? heurs[ddHeur.idx] : string("-"));
                    saveStatus = ok ? ("Saved to " + outPath) : ("[ERROR] Failed to save !");
                }
                y += 36;
                if (!saveStatus.empty()) {
                    Color sc = saveStatus.rfind("[ERROR]", 0) == 0 ? BAD : OK;
                    //wrap path kalau panjang
                    string s = saveStatus;
                    size_t pos = 0;
                    while (pos < s.size()) {
                        size_t take = min((size_t)34, s.size() - pos);
                        DrawText(s.substr(pos, take).c_str(), 20, y, 12, sc);
                        pos += take;
                        y += 14;
                    }
                }
            }
        } else {
            DrawText("Pick a file and click Solve.", 20, y, 14, TEXT_DIM);
            y += 24;
            DrawText("Keys: <- -> step, Space play,", 20, y, 12, TEXT_DIM); y += 16;
            DrawText("Home/End jump first/last.", 20, y, 12, TEXT_DIM);
        }

        //grid area
        int gridAreaX = SIDE_W + 20;
        int gridAreaY = 20;
        int gridAreaW = gW - SIDE_W - 40;
        int gridAreaH = gH - 40;

        if (boardLoaded) {
            int sz = min(gridAreaW / board.m, gridAreaH / board.n);
            if (sz < 8) sz = 8;
            int totalW = sz * board.m;
            int totalH = sz * board.n;
            int gx = gridAreaX + (gridAreaW - totalW) / 2;
            int gy = gridAreaY + (gridAreaH - totalH) / 2;

            //state aktif: kalau solved pakai trace[curStep], else state awal
            State st;
            if (solved && sol.found) st = sol.trace[curStep];
            else { st.player = board.start; st.nextDigit = 0; }

            //gambar tile
            for (int i = 0; i < board.n; i++) {
                for (int j = 0; j < board.m; j++) {
                    char ch = board.grid[i][j];
                    //start cell di-render sebagai path biasa
                    if (ch == 'Z') ch = '*';
                    DrawTile(gx + j * sz, gy + i * sz, sz, ch, st.nextDigit);
                }
            }
            //player
            DrawPlayer(gx, gy, sz, st.player.r, st.player.c);

            //label step
            if (solved && sol.found) {
                char buf[64];
                if (curStep == 0) snprintf(buf, sizeof(buf), "Initial");
                else snprintf(buf, sizeof(buf), "Step %d : %c", curStep, sol.moves[curStep - 1]);
                DrawText(buf, gridAreaX, gridAreaY, 18, TEXT);
            }
        } else {
            const char* msg = "No board loaded.";
            int fs = 20;
            int tw = MeasureText(msg, fs);
            DrawText(msg, gridAreaX + (gridAreaW - tw) / 2, gridAreaY + gridAreaH / 2 - fs / 2, fs, TEXT_DIM);
        }

        //draw popup terakhir supaya nutupin elemen lain
        DrawDropdownPopup(rFile, ddFile, files);
        DrawDropdownPopup(rAlgo, ddAlgo, algos);
        if (heurEnabled) DrawDropdownPopup(rHeur, ddHeur, heurs);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
