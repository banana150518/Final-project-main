#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>
#include "GameEngine.h"

using std::string;
using std::vector;

// ═════════════════════════════════════════════════════════════════════════════
//  TEXTURE CACHE  (โหลดครั้งเดียว, ใช้ซ้ำทุก frame)
// ═════════════════════════════════════════════════════════════════════════════
inline sf::Texture* getPlayerTexture(const string& path)
{
    if (path.empty()) return nullptr;
    static std::map<string, std::unique_ptr<sf::Texture>> cache;
    auto it = cache.find(path);
    if (it != cache.end()) return it->second.get();

    auto tex = std::make_unique<sf::Texture>();
    if (!tex->loadFromFile(path)) return nullptr;
    tex->setSmooth(true);
    return cache.emplace(path, std::move(tex)).first->second.get();
}

// ═════════════════════════════════════════════════════════════════════════════
//  RPG MAKER ANIME PALETTE
// ═════════════════════════════════════════════════════════════════════════════
namespace RPG {
    constexpr auto BG         = sf::Color(13,  13,  26);
    constexpr auto PANEL      = sf::Color(26,  26,  46);
    constexpr auto PANEL2     = sf::Color(22,  33,  62);
    constexpr auto CELL_NORM  = sf::Color(30,  42,  58);
    constexpr auto CELL_WALL  = sf::Color(58,  21,  21);
    constexpr auto CELL_TRAP  = sf::Color(52,  22,  14);
    constexpr auto CELL_CURE  = sf::Color(13,  42,  26);
    constexpr auto CELL_QST   = sf::Color(30,  30,  14);
    constexpr auto CELL_GAS   = sf::Color(42,  26,  10);

    constexpr auto BORDER     = sf::Color(74,  74,  255);
    constexpr auto BORDER2    = sf::Color(136, 136, 255);
    constexpr auto GOLD       = sf::Color(255, 215, 0);
    constexpr auto GOLD_DIM   = sf::Color(180, 140, 0);
    constexpr auto SHADOW     = sf::Color(0,   0,   0,  200);

    constexpr auto GREEN      = sf::Color(0,   255, 136);
    constexpr auto RED        = sf::Color(255, 68,  102);
    constexpr auto POISON_COL = sf::Color(68,  255, 68);
    constexpr auto STUN_COL   = sf::Color(255, 200, 50);
    constexpr auto TEAL       = sf::Color(0,   200, 180);
    constexpr auto WHITE      = sf::Color(232, 232, 255);
    constexpr auto MUTED      = sf::Color(160, 165, 200);

    constexpr auto TRAP_COL   = sf::Color(255, 100, 60);
    constexpr auto CURE_COL   = sf::Color(60,  255, 140);
    constexpr auto QST_COL    = sf::Color(255, 230, 60);
    constexpr auto WALL_COL   = sf::Color(120, 120, 130);
    constexpr auto GAS_COL    = sf::Color(255, 160, 50);
}

// ═════════════════════════════════════════════════════════════════════════════
//  PRIMITIVE HELPERS
// ═════════════════════════════════════════════════════════════════════════════

inline void drawRpgPanel(sf::RenderWindow& win,
                         float x, float y, float w, float h,
                         sf::Color fill, sf::Color border,
                         bool pixelShadow = true)
{
    if (pixelShadow) {
        sf::RectangleShape sh(sf::Vector2f(w, h));
        sh.setPosition({ x + 3, y + 3 });
        sh.setFillColor(sf::Color(0, 0, 0, 180));
        win.draw(sh);
    }
    sf::RectangleShape outer(sf::Vector2f(w, h));
    outer.setPosition({ x, y });
    outer.setFillColor(border);
    win.draw(outer);
    sf::RectangleShape hi(sf::Vector2f(w - 6, h - 6));
    hi.setPosition({ x + 3, y + 3 });
    hi.setFillColor(sf::Color(border.r / 3, border.g / 3, border.b / 3, 160));
    win.draw(hi);
    sf::RectangleShape inner(sf::Vector2f(w - 8, h - 8));
    inner.setPosition({ x + 4, y + 4 });
    inner.setFillColor(fill);
    win.draw(inner);
    sf::RectangleShape shine(sf::Vector2f(w - 8, 2));
    shine.setPosition({ x + 4, y + 4 });
    shine.setFillColor(sf::Color(border.r, border.g, border.b, 40));
    win.draw(shine);
}

inline void drawText(sf::RenderWindow& win, sf::Font& font,
                     const string& str, float x, float y,
                     unsigned size, sf::Color col, bool bold = false)
{
    sf::Text t(font, sf::String::fromUtf8(str.begin(), str.end()), size);
    t.setFillColor(col);
    t.setPosition({ x, y });
    if (bold) t.setStyle(sf::Text::Bold);
    win.draw(t);
}

inline void drawTextCentre(sf::RenderWindow& win, sf::Font& font,
                           const string& str, float cx, float y,
                           unsigned size, sf::Color col, bool bold = false)
{
    sf::Text t(font, sf::String::fromUtf8(str.begin(), str.end()), size);
    t.setFillColor(col);
    if (bold) t.setStyle(sf::Text::Bold);
    auto b = t.getLocalBounds();
    t.setPosition({ cx - b.size.x / 2.f, y });
    win.draw(t);
}

inline void drawHpBar(sf::RenderWindow& win,
                      float x, float y, float w, float h, float pct)
{
    sf::RectangleShape bg(sf::Vector2f(w, h));
    bg.setPosition({ x, y });
    bg.setFillColor(sf::Color(20, 20, 35));
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(sf::Color(50, 50, 80));
    win.draw(bg);

    if (pct > 0.f) {
        float fw = w * pct;
        sf::Color barCol = (pct > .5f) ? sf::Color(34, 204, 85)
                         : (pct > .25f) ? sf::Color(220, 170, 30)
                                        : sf::Color(220, 40, 60);
        sf::RectangleShape bar(sf::Vector2f(fw, h));
        bar.setPosition({ x, y });
        bar.setFillColor(barCol);
        win.draw(bar);
        sf::RectangleShape shine(sf::Vector2f(fw, h * .38f));
        shine.setPosition({ x, y });
        shine.setFillColor(sf::Color(255, 255, 255, 45));
        win.draw(shine);
    }
}

inline void drawTileLabel(sf::RenderWindow& win, sf::Font& font,
                          const string& icon, const string& sub,
                          float cx, float cy, float cs,
                          sf::Color iconCol, sf::Color subCol)
{
    float bw = cs - 4.f, bh = sub.empty() ? 13.f : 24.f;
    sf::RectangleShape bg(sf::Vector2f(bw, bh));
    bg.setPosition({ cx - bw / 2.f, cy - bh / 2.f });
    bg.setFillColor(sf::Color(0, 0, 0, 155));
    win.draw(bg);

    sf::Text ic(font, sf::String::fromUtf8(icon.begin(), icon.end()), 11);
    ic.setFillColor(iconCol);
    auto ib = ic.getLocalBounds();
    ic.setPosition({ cx - ib.size.x / 2.f, sub.empty() ? cy - ib.size.y / 2.f - 1.f : cy - 13.f });
    win.draw(ic);

    if (!sub.empty()) {
        sf::Text st(font, sf::String::fromUtf8(sub.begin(), sub.end()), 9);
        st.setFillColor(subCol);
        auto sb = st.getLocalBounds();
        st.setPosition({ cx - sb.size.x / 2.f, cy + 2.f });
        win.draw(st);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  HELPER — ตรวจว่ามีผู้เล่นอยู่ใกล้ช่อง (x,y) ≤ dist ช่องหรือไม่
// ═════════════════════════════════════════════════════════════════════════════
inline bool playerNearTile(const std::vector<Player>& players, int tx, int ty, int dist)
{
    for (auto& p : players) {
        if (!p.isAlive()) continue;
        int dx = std::abs(p.getPosition().x - tx);
        int dy = std::abs(p.getPosition().y - ty);
        if (dx <= dist && dy <= dist) return true;
    }
    return false;
}

// ═════════════════════════════════════════════════════════════════════════════
//  PLAYER CARD
// ═════════════════════════════════════════════════════════════════════════════

inline void drawPlayerCard(sf::RenderWindow& win, sf::Font& font,
                           const Player& p,
                           float x, float y, float w, float h,
                           bool isCurrent)
{
    sf::Color border = isCurrent ? RPG::GOLD : RPG::BORDER;
    drawRpgPanel(win, x, y, w, h, RPG::PANEL, border);

    float ar = 18.f;
    sf::CircleShape av(ar);
    av.setPosition({ x + 10, y + 10 });
    av.setFillColor(sf::Color(p.getColor().r / 6, p.getColor().g / 6, p.getColor().b / 6));
    av.setOutlineThickness(2.f);
    av.setOutlineColor(p.getColor());
    win.draw(av);

    sf::Texture* tex = getPlayerTexture(p.getTexturePath());
    if (tex) {
        float diam = ar * 2.f;
        auto ts = tex->getSize();
        float scale = diam / std::min((float)ts.x, (float)ts.y);
        float sw = ts.x * scale, sh = ts.y * scale;

        sf::RenderTexture rt;
        [[maybe_unused]] bool rtOk = rt.resize({ (unsigned)diam, (unsigned)diam });
        rt.clear(sf::Color::Transparent);
        sf::Sprite tmp(*tex);
        tmp.setScale({ scale, scale });
        tmp.setPosition({ ar - sw / 2.f, ar - sh / 2.f });
        rt.draw(tmp);
        rt.display();

        sf::Sprite avatarSpr(rt.getTexture());
        avatarSpr.setPosition({ x + 10, y + 10 });
        win.draw(avatarSpr);
    } else {
        sf::Text icon(font, string(1, p.getIcon()), 18);
        icon.setFillColor(p.getColor());
        icon.setStyle(sf::Text::Bold);
        auto ib = icon.getLocalBounds();
        icon.setPosition({ x + 10 + ar - ib.size.x / 2.f, y + 10 + ar - ib.size.y / 2.f - 2 });
        win.draw(icon);
    }

    drawText(win, font, p.getName(), x + 46, y + 10, 13, RPG::GOLD, true);
    drawText(win, font,
             "(" + std::to_string(p.getPosition().x) + "," + std::to_string(p.getPosition().y) + ")",
             x + 46, y + 26, 10, RPG::BORDER2);

    float hpPct = p.getHp() > 0 ? (float)p.getHp() / 25.f : 0.f;
    string hpStr = "HP  " + std::to_string(p.getHp()) + "/25";
    drawText(win, font, hpStr, x + 10, y + 52, 10, RPG::MUTED);
    drawHpBar(win, x + 10, y + 66, w - 20, 8, hpPct);

    string badge    = "READY";
    sf::Color badBg = sf::Color(10, 50, 25, 200);
    sf::Color badBd = RPG::GREEN;
    sf::Color badTx = RPG::GREEN;

    if      (!p.isAlive())   { badge = "DEAD";   badBg = {50,10,15,200}; badBd = RPG::RED;        badTx = RPG::RED;        }
    else if (!p.canMove())   { badge = "STUN";   badBg = {50,40,5,200};  badBd = RPG::STUN_COL;   badTx = RPG::STUN_COL;   }
    else if (p.isPoisoned()) { badge = "POISON"; badBg = {10,50,10,200}; badBd = RPG::POISON_COL; badTx = RPG::POISON_COL; }

    sf::RectangleShape br(sf::Vector2f(w - 20, 18));
    br.setPosition({ x + 10, y + 80 });
    br.setFillColor(badBg);
    br.setOutlineColor(badBd);
    br.setOutlineThickness(1.f);
    win.draw(br);
    sf::RectangleShape dot(sf::Vector2f(4, 4));
    dot.setPosition({ x + 14, y + 87 });
    dot.setFillColor(badTx);
    win.draw(dot);
    drawText(win, font, badge, x + 22, y + 81, 10, badTx, true);

    if (isCurrent) {
        sf::RectangleShape glow(sf::Vector2f(w + 4, h + 4));
        glow.setPosition({ x - 2, y - 2 });
        glow.setFillColor(sf::Color(0, 0, 0, 0));
        glow.setOutlineColor(sf::Color(255, 215, 0, 80));
        glow.setOutlineThickness(3.f);
        win.draw(glow);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  BOARD GRID
// ═════════════════════════════════════════════════════════════════════════════

inline void drawBoard(sf::RenderWindow& win, sf::Font& font, GameEngine& game,
                      float bx, float by, float bsize, bool showOverlay)
{
    float cs         = bsize / 10.f;
    int   roundCount = game.getRoundCount();
    auto& board      = game.getBoard();
    auto& players    = game.getPlayers();
    // ── ใช้ getGasZones() ที่ return vector<GasZone>& ────────────────
    auto& gasZones   = game.getGasZones();

    drawRpgPanel(win, bx - 6, by - 6, bsize + 12, bsize + 12,
                 sf::Color(8, 8, 14), sf::Color(30, 30, 80));

    sf::RectangleShape gridBg(sf::Vector2f(bsize, bsize));
    gridBg.setPosition({ bx, by });
    gridBg.setFillColor(sf::Color(4, 5, 8));
    win.draw(gridBg);

    if (game.hasShrinkWarning()) {
        int nextLayer = game.getNextShrinkRound() - 4;
        if (nextLayer >= 0 && nextLayer < 5) {
            float wx = bx + nextLayer * cs, wy = by + nextLayer * cs;
            float ww = bsize - nextLayer * 2 * cs, wh = bsize - nextLayer * 2 * cs;
            sf::RectangleShape warn(sf::Vector2f(ww, wh));
            warn.setPosition({ wx, wy });
            warn.setFillColor(sf::Color(255, 80, 20, 10));
            warn.setOutlineColor(sf::Color(255, 120, 20, 160));
            warn.setOutlineThickness(2.f);
            win.draw(warn);
        }
    }

    for (int cy = 0; cy < 10; cy++) {
        for (int cx2 = 0; cx2 < 10; cx2++) {
            float px   = bx + cx2 * cs, py = by + cy * cs;
            int   type = board.getTileType(cx2, cy);
            Tile* tileObj = board.getTileObj(cx2, cy);

            // ── เช็ค gas zone ด้วย gz.x / gz.y ──────────────────────────
            bool inGas = false;
            for (auto& gz : gasZones)
                if (cx2 >= gz.x && cx2 <= gz.x + 2 &&
                    cy  >= gz.y && cy  <= gz.y + 2)
                    inGas = true;

            bool hasPlayer = false;
            int  playerIdx = -1;
            for (int i = 0; i < 4; i++) {
                auto& pl = players[i];
                if (pl.isAlive() &&
                    pl.getPosition().x == cx2 && pl.getPosition().y == cy) {
                    hasPlayer = true; playerIdx = i; break;
                }
            }

            bool antidoteNear    = false;
            bool antidoteStepped = false;
            if (type == 3 && tileObj) {
                antidoteStepped = tileObj->isSteppedOn();
                if (antidoteStepped)
                    antidoteNear = playerNearTile(players, cx2, cy, 1);
            }

            sf::Color ambientCol(0, 0, 0, 0);
            bool      hasAmbient = false;
            if (!hasPlayer) {
                for (int i = 0; i < 4; i++) {
                    auto& pl = players[i];
                    if (!pl.isAlive()) continue;
                    int dx = std::abs(pl.getPosition().x - cx2);
                    int dy = std::abs(pl.getPosition().y - cy);
                    if (dx <= 1 && dy <= 1 && (dx + dy) > 0) {
                        auto    pc = pl.getColor();
                        uint8_t a  = (dx + dy == 1) ? 110 : 65;
                        ambientCol = sf::Color(pc.r / 4, pc.g / 4, pc.b / 4, a);
                        hasAmbient = true;
                        break;
                    }
                }
            }

            sf::Color cellCol;
            if (hasPlayer) {
                auto pc = players[playerIdx].getColor();
                if (type == 3 && antidoteStepped)
                    cellCol = sf::Color(4, 22, 10);
                else
                    cellCol = sf::Color(pc.r / 6, pc.g / 6, pc.b / 6);
            } else {
                switch (type) {
                    case 1: cellCol = (roundCount >= 4)
                                ? sf::Color(40, 4, 4)
                                : sf::Color(16, 16, 18);
                            break;
                    case 2: cellCol = sf::Color(10,  5,  3); break;
                    case 3: cellCol = sf::Color( 3, 10,  6); break;
                    case 4: cellCol = sf::Color(10, 10,  3); break;
                    default:cellCol = sf::Color( 5,  7, 11); break;
                }
                if (inGas) cellCol = sf::Color(22, 9, 2);
            }

            sf::RectangleShape cell(sf::Vector2f(cs - 2, cs - 2));
            cell.setPosition({ px + 1, py + 1 });
            cell.setFillColor(cellCol);
            win.draw(cell);

            if (hasAmbient) {
                sf::RectangleShape amb(sf::Vector2f(cs - 2, cs - 2));
                amb.setPosition({ px + 1, py + 1 });
                amb.setFillColor(ambientCol);
                win.draw(amb);
            }

            if (hasPlayer) {
                auto pc = players[playerIdx].getColor();
                sf::RectangleShape glow(sf::Vector2f(cs - 2, cs - 2));
                glow.setPosition({ px + 1, py + 1 });
                glow.setFillColor(sf::Color(0, 0, 0, 0));
                glow.setOutlineColor(sf::Color(pc.r, pc.g, pc.b, 140));
                glow.setOutlineThickness(2.f);
                win.draw(glow);
            }

            if (antidoteNear) {
                sf::RectangleShape glowFill(sf::Vector2f(cs - 2, cs - 2));
                glowFill.setPosition({ px + 1, py + 1 });
                glowFill.setFillColor(sf::Color(0, 200, 80, 28));
                win.draw(glowFill);
                sf::RectangleShape glowBdr(sf::Vector2f(cs - 2, cs - 2));
                glowBdr.setPosition({ px + 1, py + 1 });
                glowBdr.setFillColor(sf::Color(0, 0, 0, 0));
                glowBdr.setOutlineColor(sf::Color(0, 255, 120, 190));
                glowBdr.setOutlineThickness(2.5f);
                win.draw(glowBdr);
                float pr = cs * 0.11f;
                sf::CircleShape pulse(pr);
                pulse.setPosition({ px + cs / 2.f - pr, py + cs / 2.f - pr });
                pulse.setFillColor(sf::Color(0, 255, 100, 110));
                win.draw(pulse);
            }

            sf::RectangleShape lh(sf::Vector2f(cs, 1));
            lh.setPosition({ px, py });
            lh.setFillColor(sf::Color(30, 35, 55, 60));
            win.draw(lh);
            sf::RectangleShape lv(sf::Vector2f(1, cs));
            lv.setPosition({ px, py });
            lv.setFillColor(sf::Color(30, 35, 55, 60));
            win.draw(lv);

            float ccx = px + cs / 2.f, ccy = py + cs / 2.f;

            if (type == 1) {
                bool scaryWall = (roundCount >= 4);
                if (scaryWall) {
                    sf::RectangleShape bloodFill(sf::Vector2f(cs - 2, cs - 2));
                    bloodFill.setPosition({ px + 1, py + 1 });
                    bloodFill.setFillColor(sf::Color(120, 8, 8, 55));
                    win.draw(bloodFill);
                }
                sf::RectangleShape wb(sf::Vector2f(cs - 2, cs - 2));
                wb.setPosition({ px + 1, py + 1 });
                wb.setFillColor(sf::Color(0, 0, 0, 0));
                wb.setOutlineColor(scaryWall
                    ? sf::Color(200, 20, 20, 210)
                    : sf::Color(45, 45, 50, 160));
                wb.setOutlineThickness(scaryWall ? 2.5f : 1.5f);
                win.draw(wb);
                sf::RectangleShape w1(sf::Vector2f(cs - 10, 2));
                w1.setPosition({ px + 5, ccy - 1 });
                w1.setFillColor(scaryWall
                    ? sf::Color(180, 15, 15, 160)
                    : sf::Color(55, 55, 60, 120));
                win.draw(w1);
                sf::RectangleShape w2(sf::Vector2f(2, cs - 10));
                w2.setPosition({ ccx - 1, py + 5 });
                w2.setFillColor(scaryWall
                    ? sf::Color(180, 15, 15, 160)
                    : sf::Color(55, 55, 60, 120));
                win.draw(w2);
                if (scaryWall) {
                    sf::RectangleShape diag(sf::Vector2f(cs - 8, 1.5f));
                    diag.setPosition({ px + 4, py + 4 });
                    diag.setFillColor(sf::Color(220, 10, 10, 90));
                    diag.setRotation(sf::degrees(45));
                    win.draw(diag);
                }
                if (showOverlay)
                    drawTileLabel(win, font, "WALL", "", ccx, ccy, cs,
                                  scaryWall ? sf::Color(255, 60, 60) : sf::Color(100, 100, 110),
                                  scaryWall ? sf::Color(255, 60, 60) : sf::Color(100, 100, 110));
            }
            else if (showOverlay && tileObj) {
                sf::Color bc = tileObj->borderColor();
                if (bc.a > 0) {
                    sf::RectangleShape bdr(sf::Vector2f(cs - 2, cs - 2));
                    bdr.setPosition({ px + 1, py + 1 });
                    bdr.setFillColor(sf::Color(0, 0, 0, 0));
                    bdr.setOutlineColor(bc);
                    bdr.setOutlineThickness(2.f);
                    win.draw(bdr);
                }
                if (inGas) {
                    sf::RectangleShape gb(sf::Vector2f(cs - 2, cs - 2));
                    gb.setPosition({ px + 1, py + 1 });
                    gb.setFillColor(sf::Color(255, 120, 0, 18));
                    gb.setOutlineColor(sf::Color(255, 150, 20, 200));
                    gb.setOutlineThickness(2.f);
                    win.draw(gb);
                    drawTileLabel(win, font, "GAS", "-5HP", ccx, ccy, cs,
                                  RPG::GAS_COL, sf::Color(255, 210, 80));
                }

                if (type == 3 && tileObj) {
                    sf::Color iconCol = antidoteStepped
                        ? sf::Color(20, 160, 70)
                        : sf::Color(80, 255, 160);
                    drawTileLabel(win, font,
                                  tileObj->overlayLine1(), tileObj->overlayLine2(),
                                  ccx, ccy, cs,
                                  iconCol, sf::Color(180, 255, 200));
                }
                else if (!tileObj->overlayLine1().empty()) {
                    drawTileLabel(win, font,
                                  tileObj->overlayLine1(), tileObj->overlayLine2(),
                                  ccx, ccy, cs,
                                  tileObj->overlayColor(), sf::Color(255, 255, 180));
                }
            }
            else if (!showOverlay && inGas) {
                sf::RectangleShape gb(sf::Vector2f(cs - 2, cs - 2));
                gb.setPosition({ px + 1, py + 1 });
                gb.setFillColor(sf::Color(255, 100, 0, 22));
                gb.setOutlineColor(sf::Color(255, 140, 20, 120));
                gb.setOutlineThickness(1.5f);
                win.draw(gb);
            }
        }
    }

    // ── วาด gas zone border ด้วย gz.x / gz.y ────────────────────────
    for (auto& gz : gasZones) {
        float gx = bx + gz.x * cs, gy = by + gz.y * cs;
        sf::RectangleShape gr(sf::Vector2f(cs * 3 - 2, cs * 3 - 2));
        gr.setPosition({ gx + 1, gy + 1 });
        gr.setFillColor(sf::Color(0, 0, 0, 0));
        gr.setOutlineColor(showOverlay ? sf::Color(255, 150, 20, 220)
                                      : sf::Color(255, 130, 20, 110));
        gr.setOutlineThickness(showOverlay ? 2.5f : 1.f);
        win.draw(gr);

        // แสดง turnsLeft ที่มุม gas zone
        if (gz.turnsLeft > 0) {
            string tStr = std::to_string(gz.turnsLeft) + "T";
            drawText(win, font, tStr, gx + 3, gy + 3, 9,
                     sf::Color(255, 200, 80, 200), true);
        }
    }

    for (auto& pop : game.getPopups()) {
        float px2 = bx + pop.tile.x * cs + cs / 2.f;
        float py2 = by + pop.tile.y * cs + cs / 2.f + pop.offsetY - cs * 0.4f;
        int   a   = (int)std::max(0.f, std::min(255.f, pop.alpha));

        sf::Text txt(font, sf::String::fromUtf8(pop.label.begin(), pop.label.end()), 11);
        txt.setFillColor(sf::Color(pop.color.r, pop.color.g, pop.color.b, (uint8_t)a));
        auto tb = txt.getLocalBounds();
        float pw = tb.size.x + 10.f, ph = tb.size.y + 6.f;

        sf::RectangleShape pill(sf::Vector2f(pw, ph));
        pill.setPosition({ px2 - pw / 2.f, py2 - 2.f });
        pill.setFillColor(sf::Color(0, 0, 0, (uint8_t)(a * .7f)));
        pill.setOutlineColor(sf::Color(pop.color.r, pop.color.g, pop.color.b, (uint8_t)a));
        pill.setOutlineThickness(1.f);
        win.draw(pill);
        txt.setPosition({ px2 - tb.size.x / 2.f, py2 });
        win.draw(txt);
    }

    int currentIdx = game.getCurrentPlayer();
    for (int i = 0; i < 4; i++) {
        auto& p = players[i];
        if (!p.isAlive()) continue;
        float px2   = bx + p.getPosition().x * cs;
        float py2   = by + p.getPosition().y * cs;
        bool  isCur = (i == currentIdx && !game.isGameOver());
        float r     = cs * .30f;

        if (isCur) {
            sf::RectangleShape hl(sf::Vector2f(cs - 2, cs - 2));
            hl.setPosition({ px2 + 1, py2 + 1 });
            hl.setFillColor(sf::Color(0, 0, 0, 0));
            hl.setOutlineThickness(2.5f);
            hl.setOutlineColor(sf::Color(255, 215, 0, 200));
            win.draw(hl);
        }

        if (isCur) {
            sf::CircleShape bloom(r * 1.8f);
            bloom.setPosition({ px2 + cs/2.f - r*1.8f, py2 + cs/2.f - r*1.8f });
            bloom.setFillColor(sf::Color(p.getColor().r, p.getColor().g, p.getColor().b, 25));
            win.draw(bloom);
        }

        sf::CircleShape halo(r * 1.3f);
        halo.setPosition({ px2 + cs/2.f - r*1.3f, py2 + cs/2.f - r*1.3f });
        halo.setFillColor(sf::Color(p.getColor().r, p.getColor().g, p.getColor().b, 45));
        win.draw(halo);

        sf::CircleShape circle(r);
        circle.setPosition({ px2 + cs/2.f - r, py2 + cs/2.f - r });
        circle.setFillColor(sf::Color(p.getColor().r/5, p.getColor().g/5, p.getColor().b/5, 240));
        circle.setOutlineThickness(isCur ? 2.5f : 1.5f);
        circle.setOutlineColor(isCur ? p.getColor()
                                     : sf::Color(p.getColor().r, p.getColor().g, p.getColor().b, 160));
        win.draw(circle);

        int   tileTypeHere = board.getTileType((int)p.getPosition().x, (int)p.getPosition().y);
        Tile* tileHere     = board.getTileObj ((int)p.getPosition().x, (int)p.getPosition().y);
        bool  onStepped    = (tileTypeHere == 3 && tileHere && tileHere->isSteppedOn());

        auto bd_dummy = sf::Text(font, string(1, p.getIcon()), (unsigned)(cs * .34f)).getLocalBounds();

        sf::Texture* ptex = getPlayerTexture(p.getTexturePath());
        if (ptex) {
            float diam = r * 2.f;
            auto  ts   = ptex->getSize();
            float sc   = diam / std::min((float)ts.x, (float)ts.y);

            sf::RenderTexture brt;
            [[maybe_unused]] bool brtOk = brt.resize({ (unsigned)diam, (unsigned)diam });
            brt.clear(sf::Color::Transparent);
            sf::Sprite btmp(*ptex);
            btmp.setScale({ sc, sc });
            float sw = ts.x * sc, sh = ts.y * sc;
            btmp.setPosition({ diam / 2.f - sw / 2.f, diam / 2.f - sh / 2.f });
            brt.draw(btmp);
            brt.display();

            sf::Sprite bspr(brt.getTexture());
            bspr.setPosition({ px2 + cs / 2.f - r, py2 + cs / 2.f - r });
            if (onStepped)
                bspr.setColor(sf::Color(150, 255, 180, 230));
            else
                bspr.setColor(sf::Color(255, 255, 255, 230));
            win.draw(bspr);
        } else {
            sf::Text glow_txt(font, string(1, p.getIcon()), (unsigned)(cs * .34f));
            glow_txt.setFillColor(sf::Color(p.getColor().r, p.getColor().g, p.getColor().b, 80));
            glow_txt.setStyle(sf::Text::Bold);
            auto bd = glow_txt.getLocalBounds();
            glow_txt.setPosition({ px2 + cs/2.f - bd.size.x/2.f + 1.f,
                                   py2 + cs/2.f - bd.size.y/2.f - 1.f });
            win.draw(glow_txt);

            sf::Color iconDrawCol = onStepped
                ? sf::Color(30, 200, 80, 230)
                : sf::Color(220, 230, 255, 230);

            sf::Text icon(font, string(1, p.getIcon()), (unsigned)(cs * .34f));
            icon.setFillColor(iconDrawCol);
            icon.setStyle(sf::Text::Bold);
            icon.setPosition({ px2 + cs/2.f - bd.size.x/2.f,
                               py2 + cs/2.f - bd.size.y/2.f - 2.f });
            win.draw(icon);
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  DICE DISPLAY
// ═════════════════════════════════════════════════════════════════════════════

inline void drawDiceDisplay(sf::RenderWindow& win, sf::Font& font,
                            GameEngine& game,
                            float bx, float by, float bsize)
{
    int rolled = game.getLastDiceRoll();
    int left   = game.getStepsLeft();
    if (rolled <= 0) return;

    float cx = bx + bsize / 2.f;
    float cy = by - 2.f;

    float pillW = 100.f, pillH = 42.f;
    sf::RectangleShape shadow(sf::Vector2f(pillW + 4, pillH + 4));
    shadow.setPosition({ cx - pillW / 2.f - 2, cy - 2 });
    shadow.setFillColor(sf::Color(0, 0, 0, 180));
    win.draw(shadow);

    sf::RectangleShape pill(sf::Vector2f(pillW, pillH));
    pill.setPosition({ cx - pillW / 2.f, cy });
    pill.setFillColor(sf::Color(20, 20, 55, 230));
    pill.setOutlineColor(RPG::GOLD);
    pill.setOutlineThickness(2.f);
    win.draw(pill);

    sf::RectangleShape shine(sf::Vector2f(pillW, pillH * .35f));
    shine.setPosition({ cx - pillW / 2.f, cy });
    shine.setFillColor(sf::Color(255, 255, 255, 18));
    win.draw(shine);

    drawTextCentre(win, font, "ROLL", cx, cy + 4, 9, RPG::GOLD_DIM);
    drawTextCentre(win, font, std::to_string(rolled), cx, cy + 14, 22, RPG::GOLD);

    if (left > 0) {
        string leftStr = "x" + std::to_string(left);
        float bpx = bx + bsize - 60.f;
        float bpy = by - 2.f;

        sf::RectangleShape lsh(sf::Vector2f(54, 38));
        lsh.setPosition({ bpx + 2, bpy + 2 });
        lsh.setFillColor(sf::Color(0, 0, 0, 160));
        win.draw(lsh);

        sf::RectangleShape lb(sf::Vector2f(54, 38));
        lb.setPosition({ bpx, bpy });
        lb.setFillColor(sf::Color(20, 50, 30, 220));
        lb.setOutlineColor(RPG::GREEN);
        lb.setOutlineThickness(2.f);
        win.draw(lb);

        drawTextCentre(win, font, "LEFT", bpx + 27, bpy + 4, 9, RPG::GREEN);
        drawTextCentre(win, font, leftStr, bpx + 27, bpy + 14, 18, RPG::GREEN);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  ROUND BADGE
// ═════════════════════════════════════════════════════════════════════════════

inline void drawRoundBadge(sf::RenderWindow& win, sf::Font& font,
                           GameEngine& game,
                           float bx, float by)
{
    int round = game.getRoundCount();

    float bw = 90.f, bh = 42.f;
    float px = bx - 6.f;
    float py = by - 2.f;

    sf::RectangleShape sh(sf::Vector2f(bw, bh));
    sh.setPosition({ px + 3, py + 3 });
    sh.setFillColor(sf::Color(0, 0, 0, 180));
    win.draw(sh);

    bool warning = game.hasShrinkWarning();
    sf::Color borderCol = warning ? sf::Color(255, 120, 20) : RPG::BORDER2;
    sf::Color fillCol   = warning ? sf::Color(50, 20, 5, 230) : sf::Color(18, 18, 50, 230);

    sf::RectangleShape box(sf::Vector2f(bw, bh));
    box.setPosition({ px, py });
    box.setFillColor(fillCol);
    box.setOutlineColor(borderCol);
    box.setOutlineThickness(2.f);
    win.draw(box);

    sf::RectangleShape s2(sf::Vector2f(bw, bh * .35f));
    s2.setPosition({ px, py });
    s2.setFillColor(sf::Color(255, 255, 255, 14));
    win.draw(s2);

    sf::Color labelCol = warning ? sf::Color(255, 160, 40) : RPG::MUTED;
    drawTextCentre(win, font, "ROUND", px + bw / 2.f, py + 4, 9, labelCol, true);

    sf::Color numCol = warning ? sf::Color(255, 180, 50) : RPG::WHITE;
    drawTextCentre(win, font, std::to_string(round), px + bw / 2.f, py + 14, 22, numCol, true);

    if (warning) {
        drawTextCentre(win, font, "WALL NEXT!", px + bw / 2.f, py + bh + 4, 10,
                       sf::Color(255, 130, 30), true);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  CARD HUD
// ═════════════════════════════════════════════════════════════════════════════

inline void drawCardHud(sf::RenderWindow& win, sf::Font& font,
                        GameEngine& game,
                        float bx, float by, float bsize)
{
    int card = game.getLastCardDrawn();
    if (card <= 0) return;

    struct CardInfo {
        string name;
        string effect;
        sf::Color col;
        bool isGood;
    };
    static const CardInfo cards[8] = {
        {},
        { "CARD 1",  "LifeSteal -5/คน  ฮีล+5(cap)",  sf::Color(255, 100, 100), true  },
        { "CARD 2",  "ฮีล+5 ตัวเอง / แจก+5 ถ้าเต็ม", sf::Color( 60, 220, 120), true  },
        { "CARD 3",  "Block คนอื่นทุกคน (>20 หลุด)",  sf::Color( 80, 160, 255), true  },
        { "CARD 4",  "Block ทุกคน รวมตัวเอง (>20)",   sf::Color(180,  80, 220), true  },
        { "CARD 5",  "-10 HP ตัวเอง",                 sf::Color(255,  80,  80), false },
        { "CARD 6",  "Drain ตัวเอง / คนอื่น +2 HP",   sf::Color(255, 160,  50), false },
        { "CARD 7",  "Block ตัวเอง (>15 หลุด)",        sf::Color(255, 200,  50), false },
    };

    const CardInfo& ci = cards[card];

    float hw = 280.f, hh = 80.f;
    float hx = bx + bsize / 2.f - hw / 2.f;
    float hy = by + 14.f;

    sf::RectangleShape sh(sf::Vector2f(hw + 4, hh + 4));
    sh.setPosition({ hx - 2, hy - 2 });
    sh.setFillColor(sf::Color(0, 0, 0, 200));
    win.draw(sh);

    sf::Color hudBorder = ci.isGood ? sf::Color(60, 220, 120) : sf::Color(255, 80, 80);
    sf::RectangleShape box(sf::Vector2f(hw, hh));
    box.setPosition({ hx, hy });
    box.setFillColor(sf::Color(10, 10, 30, 245));
    box.setOutlineColor(hudBorder);
    box.setOutlineThickness(3.f);
    win.draw(box);

    sf::RectangleShape shine(sf::Vector2f(hw, hh * .3f));
    shine.setPosition({ hx, hy });
    shine.setFillColor(sf::Color(255, 255, 255, 12));
    win.draw(shine);

    sf::RectangleShape iconBg(sf::Vector2f(50.f, hh - 10.f));
    iconBg.setPosition({ hx + 5, hy + 5 });
    iconBg.setFillColor(sf::Color(ci.col.r / 4, ci.col.g / 4, ci.col.b / 4, 200));
    iconBg.setOutlineColor(sf::Color(ci.col.r, ci.col.g, ci.col.b, 160));
    iconBg.setOutlineThickness(1.5f);
    win.draw(iconBg);

    drawTextCentre(win, font, "?", hx + 30.f, hy + 14.f, 30, ci.col, true);

    float tx = hx + 64.f;
    drawText(win, font, "CARD DRAWN", tx, hy + 6, 9, sf::Color(180, 185, 220));
    drawText(win, font, ci.name,      tx, hy + 18, 16, ci.col, true);
    drawText(win, font, ci.effect,    tx, hy + 38, 11, RPG::WHITE);

    string tag    = ci.isGood ? "GOOD" : "BAD";
    sf::Color tagC = ci.isGood ? sf::Color(60, 220, 120) : sf::Color(255, 80, 80);
    sf::RectangleShape tagBox(sf::Vector2f(40.f, 16.f));
    tagBox.setPosition({ tx, hy + 56 });
    tagBox.setFillColor(sf::Color(tagC.r / 4, tagC.g / 4, tagC.b / 4, 200));
    tagBox.setOutlineColor(tagC);
    tagBox.setOutlineThickness(1.f);
    win.draw(tagBox);
    drawTextCentre(win, font, tag, tx + 20.f, hy + 58, 9, tagC, true);

    float timerPct = std::min(1.f, game.getCardPopupTimer() / 4.0f);
    sf::RectangleShape timerBg(sf::Vector2f(hw - 4, 4.f));
    timerBg.setPosition({ hx + 2, hy + hh - 6 });
    timerBg.setFillColor(sf::Color(30, 30, 60));
    win.draw(timerBg);
    sf::RectangleShape timerBar(sf::Vector2f((hw - 4) * timerPct, 4.f));
    timerBar.setPosition({ hx + 2, hy + hh - 6 });
    timerBar.setFillColor(sf::Color(ci.col.r, ci.col.g, ci.col.b, 200));
    win.draw(timerBar);
}

// ═════════════════════════════════════════════════════════════════════════════
//  SHRINK WARNING BANNER
// ═════════════════════════════════════════════════════════════════════════════

inline void drawShrinkWarningBanner(sf::RenderWindow& win, sf::Font& font,
                                    GameEngine& game,
                                    float bx, float by, float bsize)
{
    if (!game.hasShrinkWarning()) return;

    float bw = 320.f, bh = 52.f;
    float px = bx + bsize / 2.f - bw / 2.f;
    float py = by + bsize - bh - 10.f;

    sf::RectangleShape sh(sf::Vector2f(bw + 4, bh + 4));
    sh.setPosition({ px - 2, py - 2 });
    sh.setFillColor(sf::Color(0, 0, 0, 200));
    win.draw(sh);

    sf::RectangleShape box(sf::Vector2f(bw, bh));
    box.setPosition({ px, py });
    box.setFillColor(sf::Color(60, 20, 5, 240));
    box.setOutlineColor(sf::Color(255, 130, 20));
    box.setOutlineThickness(3.f);
    win.draw(box);

    sf::RectangleShape shine(sf::Vector2f(bw, bh * .3f));
    shine.setPosition({ px, py });
    shine.setFillColor(sf::Color(255, 255, 255, 10));
    win.draw(shine);

    drawText(win, font, "!!", px + 12, py + 12, 22, sf::Color(255, 150, 30), true);
    drawText(win, font, "WALL APPROACHING!", px + 48, py + 8, 14,
             sf::Color(255, 160, 40), true);
    drawText(win, font,
             "กำแพงจะบีบในรอบที่ " + std::to_string(game.getNextShrinkRound()),
             px + 48, py + 28, 11, RPG::WHITE);
}

// ═════════════════════════════════════════════════════════════════════════════
//  WALL POPUP
// ═════════════════════════════════════════════════════════════════════════════

inline void drawWallPopup(sf::RenderWindow& win, sf::Font& font,
                          float cx, float cy)
{
    float pw = 200.f, ph = 60.f;

    sf::RectangleShape dim(sf::Vector2f(pw + 20, ph + 20));
    dim.setPosition({ cx - (pw + 20) / 2.f, cy - (ph + 20) / 2.f });
    dim.setFillColor(sf::Color(0, 0, 0, 80));
    win.draw(dim);

    sf::RectangleShape sh(sf::Vector2f(pw, ph));
    sh.setPosition({ cx - pw / 2.f + 4, cy - ph / 2.f + 4 });
    sh.setFillColor(sf::Color(0, 0, 0, 200));
    win.draw(sh);

    sf::RectangleShape box(sf::Vector2f(pw, ph));
    box.setPosition({ cx - pw / 2.f, cy - ph / 2.f });
    box.setFillColor(sf::Color(60, 15, 15, 240));
    box.setOutlineColor(RPG::RED);
    box.setOutlineThickness(3.f);
    win.draw(box);

    sf::RectangleShape s2(sf::Vector2f(pw, ph * .3f));
    s2.setPosition({ cx - pw / 2.f, cy - ph / 2.f });
    s2.setFillColor(sf::Color(255, 255, 255, 12));
    win.draw(s2);

    drawTextCentre(win, font, "! BLOCKED !", cx, cy - ph / 2.f + 8, 14, RPG::RED);
    drawTextCentre(win, font, "ชนกำแพง!", cx, cy - ph / 2.f + 26, 16, RPG::WHITE);
    drawTextCentre(win, font, "เลือกทิศทางอื่น", cx, cy - ph / 2.f + 44, 10, RPG::MUTED);
}

// ═════════════════════════════════════════════════════════════════════════════
//  LEGEND PANEL
// ═════════════════════════════════════════════════════════════════════════════

inline void drawLegend(sf::RenderWindow& win, sf::Font& font,
                       float x, float y, float w, float h)
{
    drawRpgPanel(win, x, y, w, h, RPG::PANEL, RPG::BORDER);

    sf::RectangleShape titleBar(sf::Vector2f(w - 8, 18));
    titleBar.setPosition({ x + 4, y + 4 });
    titleBar.setFillColor(sf::Color(40, 40, 100, 160));
    win.draw(titleBar);
    drawText(win, font, "MAP KEY  [H]", x + 10, y + 6, 11, RPG::GOLD, true);

    struct LI { sf::Color col; string icon; string txt; };
    vector<LI> items = {
        { RPG::TRAP_COL,  "!", "TRAP   -10HP/PSN2T/+5" },
        { RPG::CURE_COL,  "+", "CURE   Cure+Warp"       },
        { RPG::QST_COL,   "?", "CARD   7 Effects"       },
        { RPG::WALL_COL,  "X", "WALL   Blocked"          },
        { RPG::GAS_COL,   "~", "GAS    -5HP/turn [3T]"  },
    };
    float iy = y + 28;
    for (auto& it : items) {
        sf::RectangleShape dot(sf::Vector2f(10, 10));
        dot.setPosition({ x + 10, iy + 2 });
        dot.setFillColor(sf::Color(it.col.r / 3, it.col.g / 3, it.col.b / 3));
        dot.setOutlineColor(it.col);
        dot.setOutlineThickness(1.f);
        win.draw(dot);
        drawText(win, font, it.icon, x + 12, iy + 1, 9, it.col, true);
        drawText(win, font, it.txt,  x + 26, iy + 2, 9, RPG::MUTED);
        iy += 20;
    }

    iy += 4;
    sf::RectangleShape div(sf::Vector2f(w - 20, 1));
    div.setPosition({ x + 10, iy });
    div.setFillColor(RPG::BORDER);
    win.draw(div);
    iy += 6;

    drawText(win, font, "GOOD CARDS", x + 10, iy, 9, RPG::GREEN, true); iy += 16;
    vector<std::pair<sf::Color, string>> good = {
        { sf::Color(255, 100, 100), "1: LifeSteal -5/คน +cap5"  },
        { sf::Color( 60, 220, 120), "2: ฮีล+5 / แจก+5 ถ้าเต็ม" },
        { sf::Color( 80, 160, 255), "3: Block คนอื่น (>20)"      },
        { sf::Color(180,  80, 220), "4: Block ทุกคน (>20)"       },
    };
    for (auto& c : good) {
        sf::CircleShape dot(3.f);
        dot.setPosition({ x + 12, iy + 4 });
        dot.setFillColor(c.first);
        win.draw(dot);
        drawText(win, font, c.second, x + 22, iy, 9, RPG::MUTED);
        iy += 16;
    }

    iy += 2;

    drawText(win, font, "BAD CARDS", x + 10, iy, 9, RPG::RED, true); iy += 16;
    vector<std::pair<sf::Color, string>> bad = {
        { sf::Color(255,  80,  80), "5: -10HP ตัวเอง"         },
        { sf::Color(255, 160,  50), "6: Drain / คนอื่น +2"    },
        { sf::Color(255, 200,  50), "7: Block ตัวเอง (>15)"   },
    };
    for (auto& c : bad) {
        sf::CircleShape dot(3.f);
        dot.setPosition({ x + 12, iy + 4 });
        dot.setFillColor(c.first);
        win.draw(dot);
        drawText(win, font, c.second, x + 22, iy, 9, RPG::MUTED);
        iy += 16;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  LOG PANEL
// ═════════════════════════════════════════════════════════════════════════════

inline void drawLog(sf::RenderWindow& win, sf::Font& font, GameEngine& game,
                    float x, float y, float w, float h)
{
    drawRpgPanel(win, x, y, w, h, RPG::PANEL, RPG::BORDER);

    sf::RectangleShape titleBar(sf::Vector2f(w - 8, 18));
    titleBar.setPosition({ x + 4, y + 4 });
    titleBar.setFillColor(sf::Color(20, 30, 80, 180));
    win.draw(titleBar);
    drawText(win, font, "BATTLE LOG  [L]", x + 10, y + 6, 11, RPG::BORDER2, true);

    float ly = y + 30;
    const auto& log = game.getLog();
    for (const auto& line : log) {
        sf::RectangleShape strip(sf::Vector2f(2, 13));
        strip.setPosition({ x + 9, ly + 1 });
        strip.setFillColor(RPG::BORDER);
        win.draw(strip);
        drawText(win, font, line, x + 15, ly, 10, sf::Color(200, 205, 230));
        ly += 16;
        if (ly > y + h - 12) break;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  BOTTOM HUD / CONTROLS
// ═════════════════════════════════════════════════════════════════════════════

inline void drawControls(sf::RenderWindow& win, sf::Font& font, GameEngine& game,
                         float x, float y, float w, float h,
                         bool showOverlay, bool showMapKey, bool showBattleLog)
{
    drawRpgPanel(win, x, y, w, h, RPG::PANEL2, RPG::BORDER, false);

    auto& players = game.getPlayers();
    int   ci      = game.getCurrentPlayer();

    if (!game.isGameOver()) {

        struct TL { sf::Color c; string l; };
        vector<TL> tiles = {
            { sf::Color(50, 60, 80),    "NORMAL" },
            { sf::Color(160, 70, 30),   "TRAP"   },
            { sf::Color(40, 150, 80),   "CURE"   },
            { sf::Color(160, 150, 30),  "CARD"   },
            { sf::Color(100, 40, 40),   "WALL"   },
            { sf::Color(160, 100, 30),  "GAS"    },
        };
        float lx = x + 14;
        for (auto& tl : tiles) {
            sf::RectangleShape dot(sf::Vector2f(10, 10));
            dot.setPosition({ lx, y + 10 });
            dot.setFillColor(tl.c);
            dot.setOutlineColor(sf::Color(tl.c.r + 60, tl.c.g + 60, tl.c.b + 60, 180));
            dot.setOutlineThickness(1.f);
            win.draw(dot);
            drawText(win, font, tl.l, lx + 13, y + 8, 9, RPG::MUTED);
            lx += 82;
        }

        string turnStr = "TURN: " + players[ci].getName();
        drawText(win, font, turnStr, x + w / 2.f - 130, y + 32, 14, RPG::GOLD, true);

        string roundStr = "ROUND " + std::to_string(game.getRoundCount());
        sf::Color roundCol = game.hasShrinkWarning()
                             ? sf::Color(255, 140, 30)
                             : RPG::BORDER2;
        drawText(win, font, roundStr, x + w / 2.f + 20, y + 32, 14, roundCol, true);

        if (game.hasShrinkWarning()) {
            string warnStr = "WALL IN RND " + std::to_string(game.getNextShrinkRound());
            drawText(win, font, warnStr, x + w / 2.f + 130, y + 32, 11,
                     sf::Color(255, 120, 20), true);
        }

        float bx2 = x + w / 2.f - 140, by2 = y + 52, bw = 280, bh = 28;
        bool  canRoll = !game.hasRolledYet();

        sf::RectangleShape bsh(sf::Vector2f(bw, bh));
        bsh.setPosition({ bx2 + 3, by2 + 3 });
        bsh.setFillColor(sf::Color(0, 0, 0, 160));
        win.draw(bsh);
        sf::RectangleShape btn(sf::Vector2f(bw, bh));
        btn.setPosition({ bx2, by2 });
        btn.setFillColor(canRoll ? sf::Color(60, 55, 180) : sf::Color(30, 30, 70));
        btn.setOutlineColor(canRoll ? RPG::BORDER2 : sf::Color(60, 60, 100));
        btn.setOutlineThickness(2.f);
        win.draw(btn);
        sf::RectangleShape bsh2(sf::Vector2f(bw, bh * .35f));
        bsh2.setPosition({ bx2, by2 });
        bsh2.setFillColor(sf::Color(255, 255, 255, canRoll ? 20 : 8));
        win.draw(bsh2);

        string bl = canRoll
            ? "[ SPACE ]  ROLL DICE"
            : "Steps: " + std::to_string(game.getStepsLeft()) + "   [ WASD ] MOVE";
        drawTextCentre(win, font, bl, bx2 + bw / 2.f, by2 + 8, 12,
                       canRoll ? sf::Color(200, 200, 255) : RPG::TEAL);

        struct TogBtn { string label; bool on; float ox; sf::Color onCol; };
        vector<TogBtn> tbs = {
            { "[E] EFFECT",  showOverlay,   x + w - 480, RPG::GREEN    },
            { "[H] MAP KEY", showMapKey,    x + w - 320, RPG::GOLD     },
            { "[L] LOG",     showBattleLog, x + w - 160, RPG::BORDER2  },
        };
        for (auto& tb : tbs) {
            sf::Color oc  = tb.on ? tb.onCol : sf::Color(80, 80, 110);
            sf::Color bgc = tb.on ? sf::Color(10, 45, 25, 200) : sf::Color(20, 20, 40, 200);
            sf::RectangleShape ob(sf::Vector2f(140, 20));
            ob.setPosition({ tb.ox, y + 8 });
            ob.setFillColor(bgc);
            ob.setOutlineColor(oc);
            ob.setOutlineThickness(1.f);
            win.draw(ob);
            drawText(win, font, tb.label + (tb.on ? ": ON" : ": OFF"), tb.ox + 8, y + 10, 10, oc);
        }

        drawTextCentre(win, font,
                       "WASD/ARROW=MOVE   SPACE=ROLL   E=OVERLAY   H=MAP KEY   L=LOG   R=RESET",
                       x + w / 2.f, y + 88, 10, sf::Color(120, 125, 165));

    } else {
        drawTextCentre(win, font, "GAME OVER",     x + w / 2.f, y + 10, 20, RPG::GOLD);
        drawTextCentre(win, font, game.getWinner(), x + w / 2.f, y + 38, 15, RPG::GREEN);
        drawTextCentre(win, font, "PRESS  R  TO RESTART", x + w / 2.f, y + 66, 11, RPG::BORDER2);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  DIALOG BOX
// ═════════════════════════════════════════════════════════════════════════════

inline void drawDialogBox(sf::RenderWindow& win, sf::Font& font,
                          const string& speakerName,
                          const string& message,
                          char speakerIcon, sf::Color speakerCol,
                          float x, float y, float w, float h)
{
    drawRpgPanel(win, x, y, w, h, RPG::PANEL, RPG::GOLD);

    sf::RectangleShape face(sf::Vector2f(40, 40));
    face.setPosition({ x + 10, y + 10 });
    face.setFillColor(sf::Color(10, 10, 25));
    face.setOutlineColor(speakerCol);
    face.setOutlineThickness(2.f);
    win.draw(face);
    sf::Text ic(font, string(1, speakerIcon), 22);
    ic.setFillColor(speakerCol);
    auto ib = ic.getLocalBounds();
    ic.setPosition({ x + 10 + 20 - ib.size.x / 2.f, y + 10 + 20 - ib.size.y / 2.f - 2 });
    win.draw(ic);

    drawText(win, font, speakerName, x + 58, y + 10, 11, RPG::GOLD, true);
    drawText(win, font, message,     x + 58, y + 28, 12, RPG::WHITE);
}