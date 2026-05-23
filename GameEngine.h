#pragma once
#include <vector>
#include <string>
#include <deque>
#include <algorithm>
#include <cstdlib>
#include "Player.h"
#include "Board.h"
#include "types.h"

using std::vector;
using std::string;
using std::deque;

// Gas zone with lifetime
struct GasZone {
    int x, y;
    int turnsLeft; // จำนวนเทิร์นที่เหลือ (เริ่มที่ 3)
};

class GameEngine {
public:
    GameEngine() {
        // ── Renderer-only data (color) อยู่ใน Player constructor ──
        players = {
            Player("Boss",   'B', sf::Color(100, 200, 255)),
            Player("Potter", 'P', sf::Color(255, 200,  80)),
            Player("Pim",    'M', sf::Color(180, 120, 255)),
            Player("Nammon", 'N', sf::Color( 80, 220, 150)),
        };
        resetGame();
    }

    // ── Game Logic ────────────────────────────────────────────────
    void resetGame() {
        board.reset();
        for (auto& p : players) p.revive();

        vector<Point> taken;
        for (int i = 0; i < 4; i++) {
            Point pos;
            do {
                pos = { rand() % 10, rand() % 10 };
            } while (!board.isWalkable(pos.x, pos.y) ||
                     board.getTileType(pos.x, pos.y) != 0 ||
                     std::find(taken.begin(), taken.end(), pos) != taken.end());
            taken.push_back(pos);
            players[i].setPosition(pos);
        }

        turnCount = roundCount = shrinkLayer = 0;
        stepsLeft = lastDiceRoll = lastCardDrawn = currentPlayer = 0;
        hasRolled = gameOver = wallPopup = shrinkWarning = false;
        winner = "";
        wallPopupTimer = shrinkWarningTimer = cardPopupTimer = 0.f;
        gasZones.clear();
        logLines.clear();
        popups.clear();
        addLog("เริ่มเกม! ผู้เล่นแรก: " + players[currentPlayer].getName());
    }

    void rollDice() {
        if (gameOver) return;
        Player& p = players[currentPlayer];
        if (!p.isAlive()) { addLog(p.getName() + " ตายแล้ว ข้ามเทิร์น"); nextTurn(); return; }
        if (!p.canMove()) {
            if (p.tryUnblock()) addLog(p.getName() + " หลุดจากสตัน!");
            else { addLog(p.getName() + " ถูกสตัน ข้ามเทิร์น"); nextTurn(); return; }
        }
        stepsLeft = lastDiceRoll = p.rollDice();
        hasRolled = true;
        addLog(p.getName() + " ทอยได้ " + std::to_string(stepsLeft) + " ก้าว");
    }

    void movePlayer(const string& dir) {
        if (gameOver || !hasRolled || stepsLeft <= 0) return;
        Player& p = players[currentPlayer];
        if (!p.isAlive()) {
            addLog(p.getName() + " ตายแล้ว ไม่สามารถเดินได้");
            stepsLeft = 0; nextTurn(); return;
        }

        Point next = p.getPosition();
        if (dir == "W") next.y--; if (dir == "S") next.y++;
        if (dir == "A") next.x--; if (dir == "D") next.x++;

        if (!board.isWalkable(next.x, next.y)) {
            addLog("เดินไม่ได้! มีกำแพงหรือขอบบอร์ด");
            wallPopup = true; wallPopupTimer = 1.8f; return;
        }
        wallPopup = false;

        if (!p.move(dir, board)) { addLog("เดินไม่ได้!"); return; }
        stepsLeft--;

        Tile* t = board.getTile(p.getPosition().x, p.getPosition().y);
        if (t) {
            int cardDrawn = 0;
            string result = t->onStep(p, players, board, cardDrawn);
            if (cardDrawn > 0) { lastCardDrawn = cardDrawn; cardPopupTimer = 4.0f; }
            if (!result.empty()) {
                addLog(p.getName() + ": " + result);
                sf::Color popCol = (t->overlayColor() == sf::Color::White)
                                   ? sf::Color(255, 220, 80) : t->overlayColor();
                spawnPopup(p.getPosition(), result, popCol);
            }
        }

        if (!p.isAlive()) {
            addLog(p.getName() + " ตาย! ข้ามเทิร์น");
            stepsLeft = lastDiceRoll = 0; nextTurn(); return;
        }
        if (!p.canMove() && stepsLeft > 0) {
            addLog(p.getName() + " ถูกบล็อก! หมดเทิร์น");
            stepsLeft = lastDiceRoll = 0; nextTurn(); return;
        }
        if (stepsLeft == 0) { lastDiceRoll = 0; nextTurn(); }
    }

    // ── Renderer-only ─────────────────────────────────────────────
    void spawnPopup(Point tile, const string& label, sf::Color col) {
        popups.push_back({ tile, label, col, 255.f, 0.f });
    }

    void tickPopups(float dt) {
        if (wallPopup)     { wallPopupTimer     -= dt; if (wallPopupTimer     <= 0.f) wallPopup     = false; }
        if (shrinkWarning) { shrinkWarningTimer -= dt; if (shrinkWarningTimer <= 0.f) shrinkWarning = false; }
        if (cardPopupTimer > 0.f) { cardPopupTimer -= dt; if (cardPopupTimer <= 0.f) lastCardDrawn = 0; }

        for (auto& pop : popups) { pop.offsetY -= 40.f * dt; pop.alpha -= 180.f * dt; }
        popups.erase(std::remove_if(popups.begin(), popups.end(),
            [](const EffectPopup& p) { return p.alpha <= 0.f; }), popups.end());
    }

    // ── Getters: Game Logic ───────────────────────────────────────
    vector<Player>&             getPlayers()      { return players; }
    Board&                      getBoard()        { return board; }
    int    getCurrentPlayer()   const             { return currentPlayer; }
    bool   isGameOver()         const             { return gameOver; }
    string getWinner()          const             { return winner; }
    int    getStepsLeft()       const             { return stepsLeft; }
    bool   hasRolledYet()       const             { return hasRolled; }
    int    getRoundCount()      const             { return roundCount; }
    int    getNextShrinkRound() const             { return shrinkLayer >= 5 ? 0 : 4 + (shrinkLayer * 2); }

    // ── Getters: Renderer-only ────────────────────────────────────
    deque<string>&              getLog()          { return logLines; }
    // Return pairs for renderer compatibility
    vector<std::pair<int,int>>  getGasZonePairs() const {
        vector<std::pair<int,int>> pairs;
        for (auto& gz : gasZones) pairs.push_back({ gz.x, gz.y });
        return pairs;
    }
    // Keep old name working for Renderer that calls getGasZones()
    // We expose the full struct vector for internal use
    vector<GasZone>&            getGasZones()     { return gasZones; }
    vector<EffectPopup>&        getPopups()       { return popups; }
    int    getLastDiceRoll()    const             { return lastDiceRoll; }
    bool   hasWallPopup()       const             { return wallPopup; }
    bool   hasShrinkWarning()   const             { return shrinkWarning; }
    int    getLastCardDrawn()   const             { return lastCardDrawn; }
    float  getCardPopupTimer()  const             { return cardPopupTimer; }

private:
    // ── Game Logic ────────────────────────────────────────────────
    void addLog(const string& msg) {
        logLines.push_front(msg);
        if (logLines.size() > 10) logLines.pop_back();
    }

    void killPlayersOnWalls() {
        for (auto& p : players) {
            if (!p.isAlive()) continue;
            Point pos = p.getPosition();
            if (board.getTileType(pos.x, pos.y) == 1) {
                p.takeDamage(9999);
                addLog("★ " + p.getName() + " ถูกกำแพงบีบ! ตาย!");
                spawnPopup(pos, "WALL CRUSH!", sf::Color(255, 60, 60));
            }
        }
    }

    void performShrink() {
        if (shrinkLayer >= 5) return;
        addLog("!! กำแพงบีบชั้น " + std::to_string(shrinkLayer) + " !!");
        board.shrink(shrinkLayer);
        killPlayersOnWalls();
        shrinkLayer++;
        shrinkWarning = false;
    }

    void nextTurn() {
        if (gameOver) return;
        turnCount++;

        // นับจำนวน player ที่ยังมีชีวิต
        int aliveCount = 0;
        for (auto& p : players) if (p.isAlive()) aliveCount++;

        // 1 รอบ = ทุกคนที่เหลือเดินครบ
        if (aliveCount > 0 && turnCount % aliveCount == 0) {
            roundCount++;
            addLog("─── จบรอบที่ " + std::to_string(roundCount) + " ───");

            // เตือนรอบที่ 3
            if (roundCount == 3 && !shrinkWarning && shrinkLayer < 5) {
                shrinkWarning = true; shrinkWarningTimer = 5.0f;
                addLog("!! เตือน: กำแพงจะบีบในรอบหน้า !!");
            }

            // บีบทุก 2 รอบ เริ่มรอบที่ 4
            if (shrinkLayer < 5) {
                int shrinkRound = 4 + (shrinkLayer * 2);
                if (roundCount == shrinkRound) {
                    performShrink();
                    if (shrinkLayer < 5) {
                        shrinkWarning = true; shrinkWarningTimer = 5.0f;
                        addLog("!! เตือน: กำแพงจะบีบในรอบหน้า !!");
                    }
                }
            }
        }

        // ── ลด lifetime ของ gas zones และลบที่หมดอายุ ─────────────────
        tickGasZones();

        // สุ่ม gas zone ใหม่
        randomGasZone();

        for (auto& p : players) if (p.isAlive()) p.tickPoison();

        int nextIdx = currentPlayer, startIdx = currentPlayer;
        do { nextIdx = (nextIdx + 1) % 4; }
        while (!players[nextIdx].isAlive() && nextIdx != startIdx);

        if (!players[nextIdx].isAlive()) { gameOver = true; winner = "ไม่มีผู้ชนะ"; return; }

        checkWinner();
        if (gameOver) return;

        currentPlayer = nextIdx;
        hasRolled = false; stepsLeft = lastDiceRoll = 0;
        addLog("เทิร์น: " + players[currentPlayer].getName());
    }

    // ── ลด turnsLeft ของทุก gas zone, ลบที่ turnsLeft <= 0 ──────────
    void tickGasZones() {
        for (auto& gz : gasZones) gz.turnsLeft--;
        gasZones.erase(
            std::remove_if(gasZones.begin(), gasZones.end(),
                [](const GasZone& gz) { return gz.turnsLeft <= 0; }),
            gasZones.end());
    }

    void randomGasZone() {
        if (rand() % 2 == 0) return;
        int ox = rand() % 8, oy = rand() % 8;
        // สร้าง gas zone ใหม่ lifetime = 3 เทิร์น
        gasZones.push_back({ ox, oy, 3 });
        addLog("แก๊สพิษที่ (" + std::to_string(ox) + "," + std::to_string(oy) + ")! [3 เทิร์น]");
        for (auto& p : players) {
            if (!p.isAlive()) continue;
            Point pos = p.getPosition();
            if (pos.x >= ox && pos.x <= ox + 2 && pos.y >= oy && pos.y <= oy + 2) {
                p.takeDamage(5);
                addLog(p.getName() + " โดนแก๊ส -5 HP!");
                spawnPopup(pos, "GAS  -5HP", sf::Color(255, 180, 60));
            }
        }
    }

    void checkWinner() {
        vector<Player*> alive;
        for (auto& p : players) if (p.isAlive()) alive.push_back(&p);
        if (alive.size() == 1)  { gameOver = true; winner = alive[0]->getName() + " ชนะ!"; addLog("จบเกม - " + winner); }
        else if (alive.empty()) { gameOver = true; winner = "ไม่มีผู้ชนะ"; }
    }

    // ── Core state ────────────────────────────────────────────────
    vector<Player> players;
    Board          board;
    int  turnCount = 0, roundCount = 0, shrinkLayer = 0;
    int  currentPlayer = 0, stepsLeft = 0, lastDiceRoll = 0;
    bool hasRolled = false, gameOver = false;
    string winner;

    // ── Renderer-only state ───────────────────────────────────────
    deque<string>   logLines;
    vector<GasZone> gasZones;   // ← เปลี่ยนจาก pair เป็น GasZone struct
    vector<EffectPopup> popups;
    int   lastCardDrawn      = 0;
    float cardPopupTimer     = 0.f;
    bool  wallPopup          = false;
    float wallPopupTimer     = 0.f;
    bool  shrinkWarning      = false;
    float shrinkWarningTimer = 0.f;
};