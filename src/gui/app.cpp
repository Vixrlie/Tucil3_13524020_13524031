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

//flag konsumsi klik per-frame, dipakai supaya popup dropdown / modal nutup widget di belakangnya
static bool gClickConsumed = false;
static bool ClickPressed() { return !gClickConsumed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON); }

//pecah teks jadi baris2 yg muat di width pixel tertentu (word-wrap, fallback char-wrap).
//pakai measure incremental supaya O(n) dlm jumlah karakter, bukan O(n^2).
static vector<string> WrapText(const string& text, int fontSize, int maxWidth) {
    vector<string> lines;
    string line;
    int lineW = 0;
    string word;
    int wordW = 0;
    int spaceW = MeasureText(" ", fontSize);

    auto charW = [&](char c) {
        char s[2] = { c, 0 };
        return MeasureText(s, fontSize);
    };

    auto pushWord = [&]() {
        if (word.empty()) return;
        int candW = line.empty() ? wordW : lineW + spaceW + wordW;
        if (candW <= maxWidth) {
            if (line.empty()) { line = word; lineW = wordW; }
            else { line += ' '; line += word; lineW += spaceW + wordW; }
        } else {
            if (!line.empty()) { lines.push_back(line); line.clear(); lineW = 0; }
            if (wordW > maxWidth) {
                //hard-wrap per char incremental
                string chunk;
                int chunkW = 0;
                for (char ch : word) {
                    int cw = charW(ch);
                    if (chunkW + cw > maxWidth && !chunk.empty()) {
                        lines.push_back(chunk);
                        chunk.clear();
                        chunkW = 0;
                    }
                    chunk += ch;
                    chunkW += cw;
                }
                line = chunk;
                lineW = chunkW;
            } else {
                line = word;
                lineW = wordW;
            }
        }
        word.clear();
        wordW = 0;
    };

    for (char ch : text) {
        if (ch == '\n') {
            pushWord();
            lines.push_back(line);
            line.clear(); lineW = 0;
        } else if (ch == ' ' || ch == '\t') {
            pushWord();
        } else {
            word += ch;
            wordW += charW(ch);
        }
    }
    pushWord();
    if (!line.empty()) lines.push_back(line);
    return lines;
}

//gambar teks ber-wrap di lebar tertentu, return tinggi yg digunakan
static int DrawWrappedText(const string& text, int x, int y, int maxWidth, int fontSize, int lineH, Color c) {
    vector<string> lines = WrapText(text, fontSize, maxWidth);
    int yy = y;
    for (const string& ln : lines) {
        DrawText(ln.c_str(), x, yy, fontSize, c);
        yy += lineH;
    }
    return yy - y;
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
    bool clicked = hover && ClickPressed();
    if (clicked) gClickConsumed = true;
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
    if (hover && ClickPressed()) {
        dd.open = !dd.open;
        gClickConsumed = true;
    }
    return dd.idx;
}
//panel popup dropdown digambar belakangan supaya nutupin elemen lain
static void DrawDropdownPopup(Rectangle r, Dropdown& dd, const vector<string>& items) {
    if (!dd.open) return;
    Vector2 mp = GetMousePosition();
    const int rowH = 30;
    int contentH = (int)items.size() * rowH;
    //batasi tinggi popup ke ruang sisa di bawah dropdown supaya tidak overflow window
    int avail = max(60, gH - (int)(r.y + r.height) - 8);
    int popH = min(contentH, avail);
    Rectangle pop = { r.x, r.y + r.height, r.width, (float)popH };

    //scroll state per-dropdown (pakai map static keyed pointer)
    static map<Dropdown*, float> scrollMap;
    float& scroll = scrollMap[&dd];
    int maxScroll = max(0, contentH - popH);
    if (CheckCollisionPointRec(mp, pop)) {
        scroll -= GetMouseWheelMove() * 30.0f;
    }
    if (scroll < 0) scroll = 0;
    if (scroll > maxScroll) scroll = (float)maxScroll;

    DrawRectangleRec(pop, PANEL);
    DrawRectangleLinesEx(pop, 1, BORDER);

    BeginScissorMode((int)pop.x, (int)pop.y, (int)pop.width, (int)pop.height);
    for (size_t i = 0; i < items.size(); i++) {
        Rectangle row = { pop.x, pop.y + (float)((int)i * rowH) - scroll, pop.width, (float)rowH };
        bool insideBox = row.y + row.height > pop.y && row.y < pop.y + pop.height;
        if (!insideBox) continue;
        bool h = CheckCollisionPointRec(mp, row) && CheckCollisionPointRec(mp, pop);
        if (h) DrawRectangleRec(row, PANEL_HI);
        DrawText(items[i].c_str(), (int)row.x + 10, (int)row.y + 7, 16, TEXT);
        if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            dd.idx = (int)i;
            dd.open = false;
            gClickConsumed = true;
        }
    }
    EndScissorMode();

    //scrollbar tipis kalau overflow
    if (maxScroll > 0) {
        const int sbW = 4;
        float trackH = pop.height - 4;
        float thumbH = trackH * (float)popH / (float)contentH;
        if (thumbH < 16) thumbH = 16;
        float thumbY = pop.y + 2 + (trackH - thumbH) * (scroll / (float)maxScroll);
        DrawRectangle((int)(pop.x + pop.width - sbW - 2), (int)pop.y + 2, sbW, (int)trackH, BORDER);
        DrawRectangle((int)(pop.x + pop.width - sbW - 2), (int)thumbY, sbW, (int)thumbH, ACCENT);
    }

    //klik di area popup tapi bukan item (mis. di scrollbar) tetap dikonsumsi
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mp, pop)) {
        gClickConsumed = true;
    }
    //klik di luar popup & box dropdown tutup popup
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(mp, pop) && !CheckCollisionPointRec(mp, r)) {
        dd.open = false;
    }
}

//slider horizontal float [0..1]
static float Slider(Rectangle r, float v, bool& dragging) {
    Vector2 mp = GetMousePosition();
    bool hover = CheckCollisionPointRec(mp, r);
    if (hover && ClickPressed()) { dragging = true; gClickConsumed = true; }
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
    //bikin folder kalau belum ada (parent dari path)
    size_t slash = path.find_last_of('/');
    if (slash != string::npos) {
        string dir = path.substr(0, slash);
        if (!dir.empty()) mkdir(dir.c_str(), 0755); //ignore error kalau sudah ada
    }
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
    SetExitKey(KEY_NULL); //jangan close window pas user tekan ESC (dipakai untuk cancel modal)

    vector<string> files = ListTestFiles();
    Dropdown ddFile, ddAlgo, ddHeur;
    vector<string> algos = { "UCS", "GBFS", "A*" };
    vector<string> heurs = { "H1", "H2", "H3" };

    bool boardLoaded = false;
    Board board;
    string loadError;

    bool solved = false;
    Solution sol;

    int curStep = 0;
    bool playing = false;
    float playSpeed = 0.5f; //0..1, mapped ke frame interval
    float playAccum = 0.0f;

    bool sliderDrag = false;
    string saveStatus;

    //save modal state
    bool saveModalOpen = false;
    string savePathBuf; //isi text input
    int saveCaret = 0;  //posisi caret (index char di buffer)
    bool savePathError = false; //tampil error inline kalau klik Save dgn path kosong

    //scroll vertikal sidebar kalau konten melewati window height
    float sidebarScroll = 0.0f;
    int lastSidebarContentH = 0;

    while (!WindowShouldClose()) {
        gW = GetScreenWidth();
        gH = GetScreenHeight();

        //clamp dropdown idx supaya gak OOB kalau jumlah items berubah
        if (ddFile.idx >= (int)files.size()) ddFile.idx = max(0, (int)files.size() - 1);
        if (ddAlgo.idx >= (int)algos.size()) ddAlgo.idx = 0;
        if (ddHeur.idx >= (int)heurs.size()) ddHeur.idx = 0;

        //input keyboard playback (skip kalau modal terbuka supaya gak mengganggu typing)
        if (solved && !saveModalOpen) {
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

        //reset flag konsumsi klik tiap frame
        gClickConsumed = false;

        //pre-pass: kalau ada popup dropdown / modal yg terbuka, dia berhak menyerap klik
        //sebelum widget biasa di-process. Logikanya cuma deteksi area; render tetap di akhir.
        {
            const int SIDE_W_PRE = 290;
            //rect dropdown harus match dgn yg dipakai render. Apply sidebar scroll juga.
            int yPre = 90 - (int)sidebarScroll;
            Rectangle prFile = { 20, (float)yPre, SIDE_W_PRE - 40, 32 };
            int yAlgo = yPre + 60;
            Rectangle prAlgo = { 20, (float)yAlgo, SIDE_W_PRE - 40, 32 };
            int yHeur = yAlgo + 60;
            Rectangle prHeur = { 20, (float)yHeur, SIDE_W_PRE - 40, 32 };
            Vector2 mp = GetMousePosition();
            auto consumeIfPopup = [&](Rectangle r, Dropdown& dd, const vector<string>& items) {
                if (!dd.open) return;
                int contentH = (int)items.size() * 30;
                int avail = max(60, gH - (int)(r.y + r.height) - 8);
                int popH = min(contentH, avail);
                Rectangle pop = { r.x, r.y + r.height, r.width, (float)popH };
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mp, pop)) {
                    gClickConsumed = true;
                }
            };
            consumeIfPopup(prFile, ddFile, files);
            consumeIfPopup(prAlgo, ddAlgo, algos);
            bool heurEnabledPre = (ddAlgo.idx != 0); //GBFS dan A* butuh pilih heuristik, UCS tidak
            if (heurEnabledPre) consumeIfPopup(prHeur, ddHeur, heurs);
            //kalau modal save terbuka, klik DI LUAR area modal dianggap dikonsumsi.
            //klik di dalam modal area dilepas supaya tombol Cancel/Save bisa kebaca.
            if (saveModalOpen && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                const int modalW = 520, modalH = 180;
                int mx2 = (gW - modalW) / 2;
                int my2 = (gH - modalH) / 2;
                Rectangle modalRect = { (float)mx2, (float)my2, (float)modalW, (float)modalH };
                if (!CheckCollisionPointRec(mp, modalRect)) gClickConsumed = true;
            }
        }

        BeginDrawing();
        ClearBackground(BG);

        //sidebar
        const int SIDE_W = 290;
        DrawRectangle(0, 0, SIDE_W, gH, PANEL);
        DrawLine(SIDE_W, 0, SIDE_W, gH, BORDER);

        //judul tetap fixed di atas, tidak ikut scroll
        DrawText("Ice Sliding Puzzle", 20, 18, 20, TEXT);
        DrawText("Solver", 20, 42, 14, TEXT_DIM);

        //wheel scroll sidebar (kalau mouse di sidebar dan tidak overlap dgn area scrollable lain)
        Vector2 mpSb = GetMousePosition();
        bool mouseInSidebar = mpSb.x < SIDE_W && mpSb.y > 70;
        if (mouseInSidebar) {
            int maxScrollSb = max(0, lastSidebarContentH - (gH - 70));
            sidebarScroll -= GetMouseWheelMove() * 30.0f;
            if (sidebarScroll < 0) sidebarScroll = 0;
            if (sidebarScroll > maxScrollSb) sidebarScroll = (float)maxScrollSb;
        }

        //scissor untuk konten scrollable sidebar (di bawah judul)
        BeginScissorMode(0, 70, SIDE_W, gH - 70);
        int y = 90 - (int)sidebarScroll;
        Rectangle rFile = { 20, (float)y, SIDE_W - 40, 32 };
        DrawDropdown(rFile, ddFile, files, "Input file");
        y += 60;

        Rectangle rAlgo = { 20, (float)y, SIDE_W - 40, 32 };
        DrawDropdown(rAlgo, ddAlgo, algos, "Algorithm");
        y += 60;

        Rectangle rHeur = { 20, (float)y, SIDE_W - 40, 32 };
        bool heurEnabled = (ddAlgo.idx != 0); //GBFS dan A* butuh pilih heuristik
        if (heurEnabled) {
            DrawDropdown(rHeur, ddHeur, heurs, "Heuristic");
        } else {
            //UCS ga butuh heuristik
            DrawText("Heuristic", (int)rHeur.x, (int)rHeur.y - 18, 14, TEXT_DIM);
            DrawRectangleRec(rHeur, PANEL);
            DrawRectangleLinesEx(rHeur, 1, BORDER);
            DrawText("(unused for UCS)", (int)rHeur.x + 10, (int)rHeur.y + 8, 16, TEXT_DIM);
            ddHeur.open = false;
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
                //dispatch ke solver sesuai algoritma terpilih
                if (ddAlgo.idx == 0) sol = SolveUCS(board);
                else if (ddAlgo.idx == 1) sol = SolveGBFS(board, ddHeur.idx + 1);
                else if (ddAlgo.idx == 2) sol = SolveAStar(board, ddHeur.idx + 1);
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
                DrawText("Status     : FOUND", 20, y, 14, OK); y += 20;
                //moves bisa panjang, wrap
                DrawText("Moves :", 20, y, 14, TEXT); y += 18;
                y += DrawWrappedText(sol.moves, 20, y, SIDE_W - 40, 14, 16, TEXT) + 4;
                snprintf(buf, sizeof(buf), "Total cost : %d", sol.cost);
                DrawText(buf, 20, y, 14, TEXT); y += 20;
            } else {
                DrawText("Status     : NOT FOUND", 20, y, 14, BAD); y += 20;
            }
            snprintf(buf, sizeof(buf), "Iterations : %d", sol.iterations);
            DrawText(buf, 20, y, 14, TEXT); y += 20;
            snprintf(buf, sizeof(buf), "Exec time  : %s", FormatExecTime(sol.execUs).c_str());
            DrawText(buf, 20, y, 14, TEXT); y += 28;

            //playback controls hanya kalau ada solusi
            if (sol.found) {
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

            }

            //save tetap ditampilkan walau no-solution (user mungkin mau simpan info)
            Rectangle rSave = { 20, (float)y, SIDE_W - 40, 32 };
            if (Button(rSave, "Save solution to .txt") && !files.empty()) {
                savePathBuf = "test/solution/" + files[ddFile.idx];
                saveCaret = (int)savePathBuf.size();
                savePathError = false;
                saveModalOpen = true;
                saveStatus.clear();
            }
            y += 36;
            if (!saveStatus.empty()) {
                Color sc = saveStatus.rfind("[ERROR]", 0) == 0 ? BAD : OK;
                y += DrawWrappedText(saveStatus, 20, y, SIDE_W - 40, 12, 14, sc);
            }
        } else {
            DrawText("Pick a file and click Solve.", 20, y, 14, TEXT_DIM);
            y += 24;
            DrawText("Keys: <- -> step, Space play,", 20, y, 12, TEXT_DIM); y += 16;
            DrawText("Home/End jump first/last.", 20, y, 12, TEXT_DIM);
            y += 16;
        }

        //hitung total tinggi konten sidebar untuk scroll calc next frame
        lastSidebarContentH = (y + (int)sidebarScroll) - 90 + 40; //margin bawah 40
        EndScissorMode();

        //scrollbar sidebar tipis di tepi
        {
            int maxScrollSb = max(0, lastSidebarContentH - (gH - 70));
            if (maxScrollSb > 0) {
                int sbW = 4;
                float trackY = 72;
                float trackH = (float)(gH - 80);
                float thumbH = trackH * (float)(gH - 70) / (float)lastSidebarContentH;
                if (thumbH < 20) thumbH = 20;
                float thumbY = trackY + (trackH - thumbH) * (sidebarScroll / (float)maxScrollSb);
                DrawRectangle(SIDE_W - sbW - 3, (int)trackY, sbW, (int)trackH, BORDER);
                DrawRectangle(SIDE_W - sbW - 3, (int)thumbY, sbW, (int)thumbH, ACCENT);
            }
        }

        //grid area
        int gridAreaX = SIDE_W + 20;
        int gridAreaY = 20;
        int gridAreaW = max(20, gW - SIDE_W - 40);
        int gridAreaH = max(20, gH - 40);

        if (boardLoaded) {
            int sz = min(gridAreaW / max(1, board.m), gridAreaH / max(1, board.n));
            if (sz < 4) sz = 4; //tetap render kecil2 daripada crash. Tile minimum 4px.
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

        //save destination modal
        if (saveModalOpen) {
            //dim background
            DrawRectangle(0, 0, gW, gH, (Color){ 0, 0, 0, 160 });

            const int modalW = 520;
            const int modalH = 180;
            int mx = (gW - modalW) / 2;
            int my = (gH - modalH) / 2;
            Rectangle modal = { (float)mx, (float)my, (float)modalW, (float)modalH };
            DrawRectangleRec(modal, PANEL);
            DrawRectangleLinesEx(modal, 2, ACCENT);

            DrawText("Save solution to .txt", mx + 20, my + 16, 18, TEXT);
            DrawText("Destination path:", mx + 20, my + 50, 14, TEXT_DIM);

            //text input
            Rectangle rInput = { (float)(mx + 20), (float)(my + 72), (float)(modalW - 40), 32 };
            DrawRectangleRec(rInput, BG);
            DrawRectangleLinesEx(rInput, 1, ACCENT);

            //clamp caret
            if (saveCaret < 0) saveCaret = 0;
            if (saveCaret > (int)savePathBuf.size()) saveCaret = (int)savePathBuf.size();

            //insert karakter di posisi caret
            int ch = GetCharPressed();
            while (ch > 0) {
                if (ch >= 32 && ch < 127 && (int)savePathBuf.size() < 250) {
                    savePathBuf.insert(savePathBuf.begin() + saveCaret, (char)ch);
                    saveCaret++;
                }
                ch = GetCharPressed();
            }

            //paste Cmd/Ctrl + V
            bool modKey = IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER) ||
                          IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
            if (modKey && IsKeyPressed(KEY_V)) {
                const char* clip = GetClipboardText();
                if (clip) {
                    string s = clip;
                    //filter karakter aman saja
                    string clean;
                    for (char c : s) if ((unsigned char)c >= 32 && (unsigned char)c < 127) clean += c;
                    int room = 250 - (int)savePathBuf.size();
                    if ((int)clean.size() > room) clean.resize(max(0, room));
                    savePathBuf.insert(saveCaret, clean);
                    saveCaret += (int)clean.size();
                }
            }

            //caret movement & edit keys (dengan repeat saat di-hold)
            auto isRepeating = [](int key, float& accum, float initial = 0.4f, float interval = 0.04f) {
                if (IsKeyPressed(key)) { accum = 0; return true; }
                if (IsKeyDown(key)) {
                    accum += GetFrameTime();
                    if (accum > initial) { accum = initial - interval; return true; }
                }
                return false;
            };
            static float leftAccum = 0, rightAccum = 0, bsAccum = 0, delAccum = 0;

            if (isRepeating(KEY_LEFT, leftAccum)) {
                if (modKey) {
                    //word jump
                    while (saveCaret > 0 && savePathBuf[saveCaret - 1] == '/') saveCaret--;
                    while (saveCaret > 0 && savePathBuf[saveCaret - 1] != '/') saveCaret--;
                } else if (saveCaret > 0) saveCaret--;
            }
            if (isRepeating(KEY_RIGHT, rightAccum)) {
                int n = (int)savePathBuf.size();
                if (modKey) {
                    while (saveCaret < n && savePathBuf[saveCaret] == '/') saveCaret++;
                    while (saveCaret < n && savePathBuf[saveCaret] != '/') saveCaret++;
                } else if (saveCaret < n) saveCaret++;
            }
            if (IsKeyPressed(KEY_HOME)) saveCaret = 0;
            if (IsKeyPressed(KEY_END))  saveCaret = (int)savePathBuf.size();
            if (isRepeating(KEY_BACKSPACE, bsAccum) && saveCaret > 0) {
                savePathBuf.erase(saveCaret - 1, 1);
                saveCaret--;
            }
            if (isRepeating(KEY_DELETE, delAccum) && saveCaret < (int)savePathBuf.size()) {
                savePathBuf.erase(saveCaret, 1);
            }

            //draw text dengan caret + horizontal scroll supaya caret selalu visible
            int fs = 16;
            int textX = (int)rInput.x + 8;
            int textY = (int)rInput.y + 8;
            int maxW = (int)rInput.width - 16;
            //hitung lebar text sebelum caret
            string before = savePathBuf.substr(0, saveCaret);
            int caretPx = MeasureText(before.c_str(), fs);
            //scroll offset supaya caret visible: kalau caret melebihi maxW, geser kiri
            static int textScrollOff = 0;
            if (caretPx - textScrollOff > maxW - 4) textScrollOff = caretPx - (maxW - 4);
            if (caretPx - textScrollOff < 0) textScrollOff = caretPx;
            if (textScrollOff < 0) textScrollOff = 0;

            BeginScissorMode((int)rInput.x + 4, (int)rInput.y, (int)rInput.width - 8, (int)rInput.height);
            DrawText(savePathBuf.c_str(), textX - textScrollOff, textY, fs, TEXT);
            //caret blink (deterministik 500ms cycle, gak goyah saat resize)
            int blinkPhase = (int)(GetTime() * 2.0) & 1;
            if (blinkPhase == 0) {
                int cx = textX + caretPx - textScrollOff;
                DrawRectangle(cx, textY, 2, fs, TEXT);
            }
            EndScissorMode();

            //inline error (gambar SEBELUM tombol supaya selalu visible saat doSave gagal)
            if (savePathError) {
                DrawText("[ERROR] Path is empty !", mx + 20, my + modalH - 78, 14, BAD);
            }

            //tombol Cancel & Save
            Rectangle rCancel = { (float)(mx + modalW - 220), (float)(my + modalH - 50), 90, 34 };
            Rectangle rOk     = { (float)(mx + modalW - 120), (float)(my + modalH - 50), 100, 34 };
            bool doSave = false;
            if (Button(rCancel, "Cancel")) saveModalOpen = false;
            if (Button(rOk, "Save")) doSave = true;

            //hotkey
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) doSave = true;
            if (IsKeyPressed(KEY_ESCAPE)) saveModalOpen = false;

            if (doSave) {
                if (savePathBuf.empty()) {
                    savePathError = true;
                } else {
                    bool ok = SaveSolutionTxt(savePathBuf, board, sol,
                                              algos[ddAlgo.idx],
                                              heurEnabled ? heurs[ddHeur.idx] : string("-"));
                    saveStatus = ok ? ("Saved to " + savePathBuf) : ("[ERROR] Failed to save !");
                    saveModalOpen = false;
                    savePathError = false;
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
