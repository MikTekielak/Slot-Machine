#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cmath>
#include <algorithm>
#include <SFML/Audio.hpp>

class Reel {
public:
    Reel(int x, int y, int width, int height, const std::vector<std::string>& symbols,
         std::map<std::string, std::unique_ptr<sf::Texture>>& symbolTextures)
        : x(x), y(y), width(width), height(height), symbols(symbols), symbolTextures(symbolTextures) {
        for (int i = -1; i < 3; ++i) {
            sf::Sprite sprite;
            sprite.setPosition(x, y + i * height);
            std::string symbol = symbols[rand() % symbols.size()];
            sprite.setTexture(*symbolTextures[symbol]);
            symbolNames.push_back(symbol);
            sprites.push_back(sprite);
        }
        position = 0;
        spinning = false;
    }

    void startSpinning(sf::Time duration) {
        this->spinDuration = duration;
        timeSpun = sf::Time::Zero;
        spinning = true;
        int baseNumberOfSymbols = 10;
        int randomSymbols = rand() % 5;
        int numberOfSymbolsToSpin = baseNumberOfSymbols + randomSymbols;
        totalDistance = numberOfSymbolsToSpin * height;
        initialSpeed = totalDistance / spinDuration.asSeconds();
        float maxSpeed = 50.0f;
        if (initialSpeed > maxSpeed) {
            initialSpeed = maxSpeed;
        }
    }

    void update(sf::Time deltaTime) {
        if (spinning) {
            timeSpun += deltaTime;
            float t = timeSpun.asSeconds() / spinDuration.asSeconds();
            if (t > 1.0f) t = 1.0f;
            float easedT = 0.5f * (1 - cos(t * 3.14159265f));
            position = -totalDistance * easedT;
            while (position <= -height) {
                position += height;
                sprites.pop_back();
                symbolNames.pop_back();
                sf::Sprite sprite;
                sprite.setPosition(x, y - height - position);
                std::string symbol = symbols[rand() % symbols.size()];
                sprite.setTexture(*symbolTextures[symbol]);
                sprites.insert(sprites.begin(), sprite);
                symbolNames.insert(symbolNames.begin(), symbol);
            }
            for (size_t i = 0; i < sprites.size(); ++i) {
                sprites[i].setPosition(x, y + (i - 1) * height - position);
            }
            if (timeSpun >= spinDuration) {
                spinning = false;
                position = 0.0f;
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (size_t i = 0; i < sprites.size(); ++i) {
            if (sprites[i].getPosition().y >= y - height && sprites[i].getPosition().y <= y + 5 * height) {
                window.draw(sprites[i]);
            }
        }
    }

    bool isSpinning() const {
        return spinning;
    }

    std::vector<std::string> getVisibleSymbols() const {
        return std::vector<std::string>(symbolNames.begin() + 1, symbolNames.begin() + 4);
    }

    int getX() const { return x; }
    int getY() const { return y; }

    bool spinning;

private:
    std::vector<sf::Sprite> sprites;
    std::vector<std::string> symbolNames;
    float position;
    float initialSpeed;
    sf::Time spinDuration;
    sf::Time timeSpun;
    float totalDistance;
    int x, y;
    int width, height;
    const std::vector<std::string>& symbols;
    std::map<std::string, std::unique_ptr<sf::Texture>>& symbolTextures;
};

struct WinningLine {
    std::vector<sf::Vector2f> points;
};

void drawWinningLine(sf::RenderWindow& window, const WinningLine& line, float thickness) {
    if (line.points.size() < 2) return;

    for (size_t i = 0; i < line.points.size() - 1; ++i) {
        sf::Vector2f point1 = line.points[i];
        sf::Vector2f point2 = line.points[i + 1];

        sf::Vector2f direction = point2 - point1;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        sf::RectangleShape thickLine(sf::Vector2f(length, thickness));
        thickLine.setPosition(point1);
        thickLine.setFillColor(sf::Color::White);

        float angle = std::atan2(direction.y, direction.x) * 180 / 3.14159265f;
        thickLine.setRotation(angle);

        window.draw(thickLine);
    }
}

void checkWin(const std::vector<std::string>& symbols, std::string (&result)[3][5], const std::vector<float>& multplr, float& balance, int rollPrice, float& win, std::vector<WinningLine>& winningLines, const std::vector<Reel>& reels,sf::Sound& lineMatchSound,sf::Sound& bigLineMatchSound) {
    winningLines.clear();

    int symbolWidth = 140;
    int symbolHeight = 140;
    //Check for horizontal lines
    for (int row = 0; row < 3; ++row) {
        if (result[row][0] == result[row][1] && result[row][1] == result[row][2]) {
            WinningLine line;
            auto it = std::find(symbols.begin(), symbols.end(), result[row][0]);
            int index = std::distance(symbols.begin(), it);
            if (result[row][2] == result[row][3]) {
                if (result[row][3] == result[row][4]) {
                    win += (rollPrice * multplr[index]) * 4.5f;
                    for (int col = 0; col < 5; ++col) {
                        line.points.push_back(sf::Vector2f(reels[col].getX() + symbolWidth / 2.0f, reels[col].getY() + row * symbolHeight + symbolHeight / 2.0f));
                    }
                } else {
                    win += (rollPrice * multplr[index]) * 2.0f;
                    for (int col = 0; col < 4; ++col) {
                        line.points.push_back(sf::Vector2f(reels[col].getX() + symbolWidth / 2.0f, reels[col].getY() + row * symbolHeight + symbolHeight / 2.0f));
                    }
                }
            } else {
                win += rollPrice * multplr[index];
                for (int col = 0; col < 3; ++col) {
                    line.points.push_back(sf::Vector2f(reels[col].getX() + symbolWidth / 2.0f, reels[col].getY() + row * symbolHeight + symbolHeight / 2.0f));
                }
            }
            winningLines.push_back(line);
        }
    }
    //Check for diagonal lines
    if (result[0][0] == result[1][1] && result[1][1] == result[2][2]) {
        WinningLine line;
        auto it = std::find(symbols.begin(), symbols.end(), result[0][0]);
        int index = std::distance(symbols.begin(), it);
        line.points.push_back(sf::Vector2f(reels[0].getX() + symbolWidth / 2.0f, reels[0].getY() + symbolHeight / 2.0f));
        line.points.push_back(sf::Vector2f(reels[1].getX() + symbolWidth / 2.0f, reels[1].getY() + symbolHeight * 1.5f));
        line.points.push_back(sf::Vector2f(reels[2].getX() + symbolWidth / 2.0f, reels[2].getY() + symbolHeight * 2.5f));

        if (result[2][2] == result[1][3] && result[1][3] == result[0][4]) {
            win += (rollPrice * multplr[index]) * 4.5f;
            line.points.push_back(sf::Vector2f(reels[3].getX() + symbolWidth / 2.0f, reels[3].getY() + symbolHeight * 1.5f));
            line.points.push_back(sf::Vector2f(reels[4].getX() + symbolWidth / 2.0f, reels[4].getY() + symbolHeight / 2.0f));
        } else if (result[2][2] == result[1][3]) {
            win += (rollPrice * multplr[index]) * 2.0f;
            line.points.push_back(sf::Vector2f(reels[3].getX() + symbolWidth / 2.0f, reels[3].getY() + symbolHeight * 1.5f));
        } else {
            win += rollPrice * multplr[index];
        }
        winningLines.push_back(line);
    }

    if (result[2][0] == result[1][1] && result[1][1] == result[0][2]) {
        WinningLine line;
        auto it = std::find(symbols.begin(), symbols.end(), result[2][0]);
        int index = std::distance(symbols.begin(), it);
        line.points.push_back(sf::Vector2f(reels[0].getX() + symbolWidth / 2.0f, reels[0].getY() + symbolHeight * 2.5f));
        line.points.push_back(sf::Vector2f(reels[1].getX() + symbolWidth / 2.0f, reels[1].getY() + symbolHeight * 1.5f));
        line.points.push_back(sf::Vector2f(reels[2].getX() + symbolWidth / 2.0f, reels[2].getY() + symbolHeight / 2.0f));

        if (result[0][2] == result[1][3] && result[1][3] == result[2][4]) {
            win += (rollPrice * multplr[index]) * 4.5f;
            line.points.push_back(sf::Vector2f(reels[3].getX() + symbolWidth / 2.0f, reels[3].getY() + symbolHeight * 1.5f));
            line.points.push_back(sf::Vector2f(reels[4].getX() + symbolWidth / 2.0f, reels[4].getY() + symbolHeight * 2.5f));
        } else if (result[0][2] == result[1][3]) {
            win += (rollPrice * multplr[index]) * 2.0f;
            line.points.push_back(sf::Vector2f(reels[3].getX() + symbolWidth / 2.0f, reels[3].getY() + symbolHeight * 1.5f));
        } else {
            win += rollPrice * multplr[index];
        }
        winningLines.push_back(line);
    }
    //Check for line 1 mid 3 top 1 mid
    if (result[1][0] == result[0][1] && result[0][1] == result[0][2]) {
        WinningLine line;
        auto it = std::find(symbols.begin(), symbols.end(), result[1][0]);
        int index = std::distance(symbols.begin(), it);
        // Reel 0, middle row (row 1)
        line.points.push_back(sf::Vector2f(reels[0].getX() + symbolWidth / 2.0f,
                                           reels[0].getY() + (1 + 0.5f) * symbolHeight));
        // Reel 1, top row (row 0)
        line.points.push_back(sf::Vector2f(reels[1].getX() + symbolWidth / 2.0f,
                                           reels[1].getY() + (0 + 0.5f) * symbolHeight));
        // Reel 2, top row (row 0)
        line.points.push_back(sf::Vector2f(reels[2].getX() + symbolWidth / 2.0f,
                                           reels[2].getY() + (0 + 0.5f) * symbolHeight));

        if (result[0][2] == result[0][3]) {
            // Reel 3, top row (row 0)
            line.points.push_back(sf::Vector2f(reels[3].getX() + symbolWidth / 2.0f,
                                               reels[3].getY() + (0 + 0.5f) * symbolHeight));

            if (result[0][3] == result[1][4]) {
                win += (rollPrice * multplr[index]) * 4.5f;
                // Reel 4, middle row (row 1)
                line.points.push_back(sf::Vector2f(reels[4].getX() + symbolWidth / 2.0f,
                                                   reels[4].getY() + (1 + 0.5f) * symbolHeight));
            } else {
                win += (rollPrice * multplr[index]) * 2.0f;
            }
        } else {
            win += rollPrice * multplr[index];
        }
        winningLines.push_back(line);
    }
    //Check for line 1 mid 3 bottom 1 mid
    if (result[1][0] == result[2][1] && result[2][1] == result[2][2]) {
        WinningLine line;
        auto it = std::find(symbols.begin(), symbols.end(), result[1][0]);
        int index = std::distance(symbols.begin(), it);
        // Reel 0, middle row (row 1)
        line.points.push_back(sf::Vector2f(reels[0].getX() + symbolWidth / 2.0f,
                                           reels[0].getY() + (1 + 0.5f) * symbolHeight));
        // Reel 1, bottom row (row 2)
        line.points.push_back(sf::Vector2f(reels[1].getX() + symbolWidth / 2.0f,
                                           reels[1].getY() + (2 + 0.5f) * symbolHeight));
        // Reel 2, bottom row (row 2)
        line.points.push_back(sf::Vector2f(reels[2].getX() + symbolWidth / 2.0f,
                                           reels[2].getY() + (2 + 0.5f) * symbolHeight));

        if (result[2][2] == result[2][3]) {
            // Reel 3, bottom row (row 2)
            line.points.push_back(sf::Vector2f(reels[3].getX() + symbolWidth / 2.0f,
                                               reels[3].getY() + (2 + 0.5f) * symbolHeight));

            if (result[2][3] == result[1][4]) {
                win += (rollPrice * multplr[index]) * 4.5f;
                // Reel 4, middle row (row 1)
                line.points.push_back(sf::Vector2f(reels[4].getX() + symbolWidth / 2.0f,
                                                   reels[4].getY() + (1 + 0.5f) * symbolHeight));
            } else {
                win += (rollPrice * multplr[index]) * 2.0f;
            }
        } else {
            win += rollPrice * multplr[index];
        }
        winningLines.push_back(line);
    }
    if (win != 0) {
        std::cout << "You won: " << win << std::endl;
        balance += win;
        if(win<10) {
            lineMatchSound.play();
        }
        else {
            bigLineMatchSound.play();
        }
    }

    win = 0;
}


int main() {
    std::vector<int> weights = {30, 25,20 , 15, 10, 5};
    std::vector<float> multplr = {1, 1.5f, 2, 3, 4, 10};
    std::vector<std::string> symbols = {"c", "g", "h", "f", "d", "v"};
    float balance = 1000;
    int rollPrice = 1;
    float win = 0;
    const int rows = 3;
    const int cols = 5;
    std::string result[rows][cols];
    const int imageWidth = 140;
    const int imageHeight = 140;

    sf::RenderWindow window(sf::VideoMode(1024, 768), "Slot Machine");
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Error loading font." << std::endl;
        return -1;
    }
    sf::SoundBuffer spinBuffer;
    if (!spinBuffer.loadFromFile("reel-spin.wav")) {
        std::cout << "Error loading reel-spin sound." << std::endl;
        return -1;
    }
    sf::SoundBuffer lineMatchBuffer;
    if (!lineMatchBuffer.loadFromFile("line-match.wav")) {
        std::cout << "Error loading line-match sound." << std::endl;
        return -1;
    }
    sf::Sound lineMatchSound;
    sf::SoundBuffer bigLineMatchBuffer;
    if (!bigLineMatchBuffer.loadFromFile("big-line-match.wav")) {
        std::cout << "Error loading line-match sound." << std::endl;
        return -1;
    }
    sf::Sound bigLineMatchSound;
    bigLineMatchSound.setBuffer(bigLineMatchBuffer);
    lineMatchSound.setBuffer(lineMatchBuffer);
    sf::Sound spinSound;
    spinSound.setBuffer(spinBuffer);
    spinSound.setLoop(true);
    sf::Text balanceText;
    balanceText.setFont(font);
    balanceText.setCharacterSize(24);
    balanceText.setFillColor(sf::Color::White);
    sf::Text winText;
    winText.setFont(font);
    winText.setCharacterSize(24);
    winText.setFillColor(sf::Color::Yellow);
    sf::Texture slotMachineImage;
    if (!slotMachineImage.loadFromFile("image.png")) {
        std::cout << "Error loading background image." << std::endl;
        return -1;
    }
    sf::Texture backgroundImage;
    if (!backgroundImage.loadFromFile("background.png")) {
        std::cout << "Error loading background image." << std::endl;
        return -1;
    }
    sf::Sprite backgroundSprite(backgroundImage);

    sf::Sprite slotMachineSprite(slotMachineImage);
    float backgroundX = (window.getSize().x - slotMachineImage.getSize().x) / 2.0f;
    float backgroundY = (window.getSize().y - slotMachineImage.getSize().y) / 2.0f;
    slotMachineSprite.setPosition(backgroundX, backgroundY);
    backgroundSprite.setPosition(-460, -240);
    std::map<std::string, std::unique_ptr<sf::Texture>> symbolTextures;
    for (const auto& symbol : symbols) {
        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile("Symb" + symbol + ".png")) {
            std::cout << "Error loading texture for symbol: " << symbol << std::endl;
            return -1;
        }
        symbolTextures[symbol] = std::move(texture);
    }
    int startX = backgroundX + (slotMachineImage.getSize().x - (cols * imageWidth)) / 2;
    int startY = backgroundY + (slotMachineImage.getSize().y - (rows * imageHeight)) / 2;
    std::vector<Reel> reels;
    for (int col = 0; col < cols; ++col) {
        int x = startX + col * imageWidth;
        int y = startY;
        reels.emplace_back(x, y, imageWidth, imageHeight, symbols, symbolTextures);
    }

    sf::Clock clock;
    bool spinning = false;
    bool resultsChecked = false;
    std::vector<WinningLine> winningLines;

    while (window.isOpen()) {
        sf::Time deltaTime = clock.restart();
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && balance > 0) {
                if (event.key.code == sf::Keyboard::Space && !spinning) {
                    spinning = true;
                    resultsChecked = false;
                    balance--;
                    winText.setString("");

                    // Clear winning lines as soon as a new spin starts
                    winningLines.clear();

                    float baseDuration = 0.1f;
                    float durationIncrement = 0.6f;
                    for (int col = 0; col < cols; ++col) {
                        float duration = baseDuration + col * durationIncrement;
                        reels[col].startSpinning(sf::seconds(duration));
                    }
                    spinSound.play();
                }
            }
        }

        for (auto& reel : reels) {
            reel.update(deltaTime);
        }

        if (spinning && !resultsChecked) {
            bool allStopped = true;
            for (const auto& reel : reels) {
                if (reel.spinning) {
                    allStopped = false;
                    break;
                }
            }
            if (allStopped) {
                resultsChecked = true;
                for (int col = 0; col < cols; ++col) {
                    auto visibleSymbols = reels[col].getVisibleSymbols();
                    for (int row = 0; row < rows; ++row) {
                        result[row][col] = visibleSymbols[row];
                    }
                }
                checkWin(symbols, result, multplr, balance, rollPrice, win, winningLines, reels,lineMatchSound,bigLineMatchSound);
                if (win != 0) {

                    winText.setString("You won: " + std::to_string(win));
                    winText.setPosition(10, window.getSize().y - winText.getLocalBounds().height - 10);

                }
                spinSound.stop();

            }
        }

        if (spinning) {
            bool allFullyStopped = true;
            for (const auto& reel : reels) {
                if (reel.isSpinning()) {
                    allFullyStopped = false;
                    break;
                }
            }
            if (allFullyStopped) {
                spinning = false;
            }
        }

        balanceText.setString("Balance: " + std::to_string(balance));
        balanceText.setPosition(window.getSize().x - balanceText.getLocalBounds().width - 10, window.getSize().y - balanceText.getLocalBounds().height - 10);

        window.clear();

        for (auto& reel : reels) {
            reel.draw(window);
        }
        window.draw(backgroundSprite);
        window.draw(slotMachineSprite);

        window.draw(balanceText);
        window.draw(winText);

        for (const auto& line : winningLines) {
            drawWinningLine(window, line, 8.0f);  // Set the thickness to 5.0f or any value you prefer
        }

        window.display();
    }

    return 0;
}

