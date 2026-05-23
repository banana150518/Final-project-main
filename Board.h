#pragma once
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include "Tile.h"
#include "Player.h"

using std::vector;

class Board {
public:
    Board()  { reset(); }
    ~Board() { clear(); }

    // ── Game Logic ────────────────────────────────────────────────
    void reset() {
        clear();
        grid.resize(10, vector<Tile*>(10, nullptr));
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 10; j++)
                grid[i][j] = new NormalTile();

        vector<Point> available;
        for (int y = 0; y < 10; y++)
            for (int x = 0; x < 10; x++)
                if (!(x == 0 && y == 0) && !(x == 9 && y == 9))
                    available.push_back({x, y});

        for (int i = (int)available.size() - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            std::swap(available[i], available[j]);
        }

        int idx = 0;
        for (int k = 0; k < 4; k++, idx++) setTile(available[idx].x, available[idx].y, new TrapTile());
        for (int k = 0; k < 3; k++, idx++) setTile(available[idx].x, available[idx].y, new AntidoteTile());
        for (int k = 0; k < 4; k++, idx++) setTile(available[idx].x, available[idx].y, new QuestionableTile());
        for (int k = 0; k < 5; k++, idx++) setTile(available[idx].x, available[idx].y, new WallTile());
    }

    void  setTile(int x, int y, Tile* t) { delete grid[y][x]; grid[y][x] = t; }
    bool  isWalkable(int x, int y)  const { return inBounds(x, y) && grid[y][x]->isWalkable(); }
    int   getTileType(int x, int y) const { return inBounds(x, y) ? grid[y][x]->getType() : 1; }
    Tile* getTile(int x, int y)     const { return inBounds(x, y) ? grid[y][x] : nullptr; }

    bool isPlayerStanding(int x, int y, const vector<Player>& players) const {
        for (const auto& p : players)
            if (p.isAlive() && p.getPosition().x == x && p.getPosition().y == y)
                return true;
        return false;
    }

    void relocateTrap(int ox, int oy, const vector<Player>& players) {
        setTile(ox, oy, new NormalTile());
        auto candidates = gatherCandidates(ox, oy, players);
        if (candidates.empty()) { setTile(ox, oy, new TrapTile()); return; }
        Point dest = candidates[rand() % candidates.size()];
        setTile(dest.x, dest.y, new TrapTile());
    }

    void relocateQuestionable(int ox, int oy, const vector<Player>& players) {
        setTile(ox, oy, new NormalTile());
        auto candidates = gatherCandidates(ox, oy, players);
        if (candidates.empty()) { setTile(ox, oy, new QuestionableTile()); return; }
        Point dest = candidates[rand() % candidates.size()];
        setTile(dest.x, dest.y, new QuestionableTile());
    }

    void shrink(int layer) {
        if (layer >= 5) return;
        for (int i = layer; i < 10 - layer; i++) {
            setTile(layer,     i,         new WallTile());
            setTile(9 - layer, i,         new WallTile());
            setTile(i,         layer,     new WallTile());
            setTile(i,         9 - layer, new WallTile());
        }
    }

    // ── Renderer-only ─────────────────────────────────────────────
    Tile* getTileObj(int x, int y) const { return inBounds(x, y) ? grid[y][x] : nullptr; }

private:
    vector<vector<Tile*>> grid;

    bool inBounds(int x, int y) const { return x >= 0 && x < 10 && y >= 0 && y < 10; }

    void clear() {
        for (auto& row : grid) for (auto* t : row) delete t;
        grid.clear();
    }

    // หา candidate cells สำหรับ relocate:
    // - ต้องเป็น NormalTile (type == 0)
    // - ต้องไม่มีผู้เล่นยืนอยู่ (ทุกคนรวมถึงคนที่เพิ่งเหยียบ)
    // - พยายามเลือกช่องที่ห่างจากผู้เล่นทุกคน > 1 ช่องก่อน
    // - fallback: เลือกช่องใดก็ได้ที่ไม่มีคนยืน
    vector<Point> gatherCandidates(int ox, int oy, const vector<Player>& players) const {
        // รอบแรก: ห่างจากผู้เล่นทุกคน > 1 ช่อง
        vector<Point> candidates;
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                if (x == ox && y == oy) continue;
                if (grid[y][x]->getType() != 0) continue;
                if (isPlayerStanding(x, y, players)) continue;

                // ตรวจว่าห่างจากผู้เล่นทุกคน > 1 ช่อง
                bool tooClose = false;
                for (const auto& p : players) {
                    if (!p.isAlive()) continue;
                    if (std::abs(x - p.getPosition().x) <= 1 &&
                        std::abs(y - p.getPosition().y) <= 1) {
                        tooClose = true;
                        break;
                    }
                }
                if (!tooClose) candidates.push_back({x, y});
            }
        }
        if (!candidates.empty()) return candidates;

        // fallback: ไม่เช็ค distance — แค่ไม่มีคนยืน
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                if (x == ox && y == oy) continue;
                if (grid[y][x]->getType() != 0) continue;
                if (isPlayerStanding(x, y, players)) continue;
                candidates.push_back({x, y});
            }
        }
        return candidates;
    }
};

// ── TrapTile::onStep (impl ต้องอยู่หลัง Board) ───────────────────
inline string TrapTile::onStep(Player& p, std::vector<Player>& players, Board& board, int&) {
    p.takeDamage(10);
    p.setPoisoned(true, 2);
    p.heal(5);
    Point pos = p.getPosition();
    // ส่ง players ทั้งหมดเพื่อให้ gatherCandidates เช็คทุกตำแหน่ง
    board.relocateTrap(pos.x, pos.y, players);
    return "TRAP  -10HP / Poison 2T / +5HP  [relocating...]";
}

// ── QuestionableTile::onStep ──────────────────────────────────────
inline string QuestionableTile::onStep(Player& p, std::vector<Player>& players, Board& board, int& cardDrawn) {
    int card = (rand() % 7) + 1;
    cardDrawn = card;
    string msg;
    applyCard(card, p, players, msg);
    Point pos = p.getPosition();
    // ส่ง players ทั้งหมดเพื่อให้ gatherCandidates เช็คทุกตำแหน่ง
    board.relocateQuestionable(pos.x, pos.y, players);
    return "? CARD #" + std::to_string(card) + "  " + msg + "  [relocating...]";
}