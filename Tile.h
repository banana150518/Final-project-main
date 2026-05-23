#pragma once
#include <string>
#include <vector>
#include <cstdlib>
#include <SFML/Graphics/Color.hpp>
#include "Player.h"
#include "types.h"

using std::string;

class Board;

// ── Base Tile ─────────────────────────────────────────────────────
class Tile {
public:
    virtual ~Tile() {}

    // ── Game Logic ────────────────────────────────────────────────
    virtual string onStep(Player& p, std::vector<Player>& all,
                          Board& board, int& cardDrawn) = 0;
    virtual bool   isWalkable() const { return true; }
    virtual string getName()    const { return "?"; }
    virtual int    getType()    const { return 0; }

    // ── Renderer-only ─────────────────────────────────────────────
    virtual string    overlayLine1() const { return ""; }
    virtual string    overlayLine2() const { return ""; }
    virtual sf::Color overlayColor() const { return sf::Color::White; }
    virtual sf::Color borderColor()  const { return sf::Color(0, 0, 0, 0); }
    // สำหรับ antidote proximity glow — tile อื่นๆ return false เสมอ
    virtual bool      isSteppedOn()  const { return false; }
};

// ── NormalTile ────────────────────────────────────────────────────
class NormalTile : public Tile {
public:
    string onStep(Player&, std::vector<Player>&, Board&, int&) override { return ""; }
    string getName() const override { return "Normal"; }
    int    getType() const override { return 0; }
};

// ── WallTile ──────────────────────────────────────────────────────
class WallTile : public Tile {
public:
    string onStep(Player&, std::vector<Player>&, Board&, int&) override { return ""; }
    bool   isWalkable() const override { return false; }
    string getName()    const override { return "Wall"; }
    int    getType()    const override { return 1; }

    string    overlayLine1() const override { return getName(); }
    sf::Color overlayColor() const override { return sf::Color(100, 100, 110); }   // ← เทาจาง ไม่เด่น
    sf::Color borderColor()  const override { return sf::Color(60, 60, 70, 160); } // ← เทาเข้ม
};

// ── TrapTile ──────────────────────────────────────────────────────
class TrapTile : public Tile {
public:
    string onStep(Player& p, std::vector<Player>&, Board& board, int&) override;
    string getName() const override { return "Trap"; }
    int    getType() const override { return 2; }

    string    overlayLine1() const override { return getName(); }
    string    overlayLine2() const override { return "-10HP/PSN2T/+5HP"; }
    sf::Color overlayColor() const override { return sf::Color(255, 160, 80); }
    sf::Color borderColor()  const override { return sf::Color(255, 120, 30, 230); }
};

// ── AntidoteTile ──────────────────────────────────────────────────
class AntidoteTile : public Tile {
public:
    string onStep(Player& p, std::vector<Player>&, Board&, int&) override {
        revealed  = true;
        steppedOn = true;   // มีคนเหยียบแล้ว → ไอคอนเปลี่ยนเป็นเขียวเข้ม
        p.setPoisoned(false, 0);
        return "ANTIDOTE  หายพิษ!";
    }
    string getName()     const override { return "Antidote"; }
    int    getType()     const override { return 3; }
    bool   isRevealed()  const          { return revealed;   }
    bool   isSteppedOn() const          { return steppedOn;  }

    // โชว์เสมอใน debug/overlay mode (ไม่รอ revealed)
    string    overlayLine1() const override { return "CURE"; }
    string    overlayLine2() const override { return "Cure+Warp"; }

    // เขียวเข้มหลังเหยียบ, เขียวสว่างปกติ
    sf::Color overlayColor() const override {
        return steppedOn ? sf::Color(20, 160, 70)
                         : sf::Color(80, 255, 160);
    }
    sf::Color borderColor() const override {
        return steppedOn ? sf::Color(20, 180, 80, 255)
                         : sf::Color(60, 255, 130, 230);
    }

private:
    bool revealed  = false;
    bool steppedOn = false;
};

// ── Card Logic ────────────────────────────────────────────────────
inline void applyCard(int card, Player& p, std::vector<Player>& all, string& msg) {
    switch (card) {
        case 1: {
            int drained = 0;
            for (auto& o : all)
                if (o.getName() != p.getName() && o.isAlive()) { o.takeDamage(5); drained += 5; }
            int gain = std::min(drained, 5);
            p.heal(gain);
            msg = "Life Steal -5/คน  ฮีล +" + std::to_string(gain) + " (cap 5)";
            break;
        }
        case 2: {
            int missing = 50 - p.getHp();
            if (missing > 5) { p.heal(5); msg = "ฮีล +5 HP ตัวเอง"; }
            else {
                for (auto& o : all)
                    if (o.getName() != p.getName() && o.isAlive()) o.heal(5);
                msg = "เลือดเกือบเต็ม! แจก +5 ให้ทุกคนแทน";
            }
            break;
        }
        case 3: {
            for (auto& o : all)
                if (o.getName() != p.getName() && o.isAlive())
                    { o.setMovable(false); o.setBlockDiceTarget(20); }
            msg = "บล็อกทุกคนอื่น (ต้องทอย >20)";
            break;
        }
        case 4: {
            for (auto& o : all)
                if (o.isAlive()) { o.setMovable(false); o.setBlockDiceTarget(20); }
            msg = "บล็อกทุกคน รวมตัวเอง (ต้องทอย >20)";
            break;
        }
        case 5: { p.takeDamage(10); msg = "-10 HP ตัวเอง"; break; }
        case 6: {
            int cnt = 0;
            for (auto& o : all) if (o.getName() != p.getName() && o.isAlive()) cnt++;
            int drain = cnt * 2;
            p.takeDamage(std::min(drain, p.getHp()));
            for (auto& o : all) if (o.getName() != p.getName() && o.isAlive()) o.heal(2);
            msg = "Drain -" + std::to_string(drain) + " ตัวเอง / คนอื่น +2";
            break;
        }
        case 7: { p.setMovable(false); p.setBlockDiceTarget(15); msg = "บล็อกตัวเอง (ต้องทอย >15)"; break; }
        default: msg = "???";
    }
}

// ── QuestionableTile ──────────────────────────────────────────────
class QuestionableTile : public Tile {
public:
    string onStep(Player& p, std::vector<Player>& all, Board& board, int& cardDrawn) override;
    string getName() const override { return "?"; }
    int    getType() const override { return 4; }

    string    overlayLine1() const override { return "? CARD"; }
    string    overlayLine2() const override { return "Random"; }
    sf::Color overlayColor() const override { return sf::Color(255, 240, 80); }
    sf::Color borderColor()  const override { return sf::Color(255, 230, 50, 230); }
};