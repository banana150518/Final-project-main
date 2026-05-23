#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "GameEngine.h"
#include "Renderer.h"

int main() {
    srand(time(0));

    sf::RenderWindow window(sf::VideoMode({ 1400, 820 }), "Potter and the gang ", sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/tahoma.ttf"))
        if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
            if (!font.openFromFile("C:/Windows/Fonts/consola.ttf")) {
                std::cerr << "Font error" << std::endl;
                return 1;
            }

    GameEngine game;

    bool showOverlay   = false;
    bool showMapKey    = false;
    bool showBattleLog = false;

    constexpr float W = 1400, H = 820;
    constexpr float leftCardW = 230, leftCardH = 130;

    const float boardLeft   = leftCardW + 30;
    const float boardRight  = W - leftCardW - 30;
    const float boardTop    = 20;
    const float boardBottom = H - 150;
    const float boardSize   = std::min(boardRight - boardLeft - 20, boardBottom - boardTop - 20);
    const float boardX      = boardLeft + (boardRight - boardLeft - boardSize) / 2;
    const float boardY      = boardTop + 10;
    const float rightPanelX = boardX + boardSize + 16;
    const float ctrlY = boardY + boardSize + 10;
    const float ctrlH = H - ctrlY - 10;

    sf::Clock frameClock;

    while (window.isOpen()) {
        float dt = frameClock.restart().asSeconds();
        game.tickPopups(dt);

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::H) showMapKey    = !showMapKey;
                if (key->code == sf::Keyboard::Key::L) showBattleLog = !showBattleLog;
                if (key->code == sf::Keyboard::Key::E) showOverlay   = !showOverlay;
                if (key->code == sf::Keyboard::Key::R) game.resetGame();
                if (key->code == sf::Keyboard::Key::Space) {
                    if (!game.isGameOver() && !game.hasRolledYet()) game.rollDice();
                }
                if (!game.isGameOver() && game.hasRolledYet() && game.getStepsLeft() > 0) {
                    string dir;
                    if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up)    dir = "W";
                    if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down)  dir = "S";
                    if (key->code == sf::Keyboard::Key::A || key->code == sf::Keyboard::Key::Left)  dir = "A";
                    if (key->code == sf::Keyboard::Key::D || key->code == sf::Keyboard::Key::Right) dir = "D";
                    if (!dir.empty()) game.movePlayer(dir);
                }
            }
        }

        window.clear(sf::Color(12, 14, 28));

        sf::RectangleShape titleBar(sf::Vector2f(W, 40));
        titleBar.setPosition({ 0, 0 });
        titleBar.setFillColor(sf::Color(18, 20, 40));
        window.draw(titleBar);
        drawText(window, font, "x  Board Battle", 16, 9, 18, sf::Color(200, 205, 230));

        auto& players = game.getPlayers();
        int   ci      = game.getCurrentPlayer();

        float cardY0 = 50, cardY1 = cardY0 + leftCardH + 12;
        drawPlayerCard(window, font, players[0], 14, cardY0, leftCardW, leftCardH, ci == 0 && !game.isGameOver());
        drawPlayerCard(window, font, players[1], 14, cardY1, leftCardW, leftCardH, ci == 1 && !game.isGameOver());
        drawPlayerCard(window, font, players[2], rightPanelX, cardY0, leftCardW, leftCardH, ci == 2 && !game.isGameOver());
        drawPlayerCard(window, font, players[3], rightPanelX, cardY1, leftCardW, leftCardH, ci == 3 && !game.isGameOver());

        float legendY = cardY1 + leftCardH + 12;
        float legendH = boardY + boardSize - legendY;
        if (showMapKey && legendH > 60)
            drawLegend(window, font, rightPanelX, legendY, leftCardW, legendH);

        float logY = legendY + (showMapKey && legendH > 60 ? legendH : 0) + 8;
        float logH = H - logY - 10;
        if (showBattleLog && logH > 40)
            drawLog(window, font, game, rightPanelX, logY, leftCardW, logH);

        drawBoard(window, font, game, boardX, boardY, boardSize, showOverlay);
        drawRoundBadge(window, font, game, boardX, boardY);

        if (!game.isGameOver() && game.hasRolledYet())
            drawDiceDisplay(window, font, game, boardX, boardY, boardSize);

        if (game.getLastCardDrawn() > 0)
            drawCardHud(window, font, game, boardX, boardY, boardSize);

        drawShrinkWarningBanner(window, font, game, boardX, boardY, boardSize);

        if (game.hasWallPopup())
            drawWallPopup(window, font, boardX + boardSize / 2.f, boardY + boardSize / 2.f);

        drawControls(window, font, game, 14, ctrlY, rightPanelX - 18, ctrlH,
                     showOverlay, showMapKey, showBattleLog);

        window.display();
    }
    return 0;
}