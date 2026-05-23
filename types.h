#pragma once
#include <string>
#include <SFML/Graphics/Color.hpp>

// ── Game Logic ────────────────────────────────────────────────────
struct Point {
    int x = 0, y = 0;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};

// ── Renderer-only ─────────────────────────────────────────────────
struct EffectPopup {
    Point       tile;
    std::string label;
    sf::Color   color;
    float       alpha   = 255.f;
    float       offsetY = 0.f;
};