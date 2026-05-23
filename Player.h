#pragma once
#include <string>
#include <cstdlib>
#include <SFML/Graphics/Color.hpp>
#include "types.h"

using std::string;

class Player {
public:
    Player(string name_, char icon_, sf::Color color_,
           string texturePath_ = "")
        : name(name_), icon(icon_), color(color_),
          texturePath(texturePath_), hp(25) {}

    // ── Game Logic ────────────────────────────────────────────────
    bool  isAlive()        const { return alive; }
    bool  isPoisoned()     const { return poisoned; }
    int   getHp()          const { return hp; }
    bool  canMove()        const { return movable; }
    int   getPoisonTurns() const { return poisonTurns; }
    Point getPosition()    const { return pos; }
    string getName()       const { return name; }

    void setPoisoned(bool p, int turns = 3) { poisoned = p; poisonTurns = turns; }
    void setMovable(bool m)                 { movable = m; }
    void setBlockDiceTarget(int t)          { blockTarget = t; }
    void setPosition(Point p)              { pos = p; }
    void revive() {
        hp = 25; alive = true;
        poisoned = false; poisonTurns = 0;
        movable = true;   blockTarget = 0;
    }

    int rollDice() { return (rand() % 6) + 1; }

    template<typename BoardT>
    bool move(const string& dir, BoardT& board) {
        if (!movable) return false;
        Point n = pos;
        if (dir == "W") n.y--;
        if (dir == "S") n.y++;
        if (dir == "A") n.x--;
        if (dir == "D") n.x++;
        if (n.x >= 0 && n.x < 10 && n.y >= 0 && n.y < 10 && board.isWalkable(n.x, n.y)) {
            pos = n; return true;
        }
        return false;
    }

    void takeDamage(int dmg) {
        hp -= dmg;
        if (hp <= 0) { hp = 0; alive = false; }
        if (hp > 25)   hp = 25;
    }

    void heal(int amount) {
        hp += amount;
        if (hp > 25) hp = 25;
        if (hp > 0 && !alive) alive = true;
    }

    void tickPoison() {
        if (!poisoned) return;
        takeDamage(2);
        if (--poisonTurns <= 0) poisoned = false;
    }

    bool tryUnblock() {
        if (blockTarget == 0) return true;
        if ((rand() % 20) + 1 >= blockTarget) {
            movable = true; blockTarget = 0; return true;
        }
        return false;
    }

    // ── Renderer-only (display data, not used in game logic) ──────
    char      getIcon()        const { return icon; }
    sf::Color getColor()       const { return color; }
    float     getHpPercent()   const { return hp / 25.0f; }
    string    getTexturePath() const { return texturePath; }

    void setTexturePath(const string& path) { texturePath = path; }

private:
    // ── Core state ────────────────────────────────────────────────
    string name;
    int    hp          = 25;
    Point  pos         { 0, 0 };
    bool   alive       = true;
    bool   poisoned    = false;
    bool   movable     = true;
    int    poisonTurns = 0;
    int    blockTarget = 0;

    // ── Renderer-only ─────────────────────────────────────────────
    char      icon;
    sf::Color color;
    string    texturePath;
};