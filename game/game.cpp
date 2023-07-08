#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <iostream>

class WebShot {
public:
    WebShot(sf::Texture& texture, const sf::Vector2f& position)
    {
        sprite.setTexture(texture);
        sprite.setPosition(position);
    }

    sf::Sprite sprite;
    float speed = 1.f;
};

class Enemy {
public:
    Enemy(sf::Texture& texture, const sf::Vector2f& position, const sf::Vector2f& size)
    {
        sprite.setTexture(texture);
        sprite.setPosition(position);
        resize(size);
    }

    sf::Sprite sprite;
    float speed = 0.6f;

    void resize(const sf::Vector2f& size)
    {
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setScale(size.x / bounds.width, size.y / bounds.height);
    }
};


class Spiderman {
public:
    Spiderman() = default; 

    Spiderman(sf::Texture& texture, const sf::Vector2f& position)
    {
        sprite.setTexture(texture);
        sprite.setPosition(position);
    }

    sf::Sprite sprite;
};

class Game {
public:
    Game()
    {
        window.create(sf::VideoMode(1920, 1080), "Spiderman Game");

        if (!backgroundTexture.loadFromFile("background.jpg"))
            throw std::runtime_error("Failed to load background texture.");

        backgroundSprite.setTexture(backgroundTexture);

        if (!webShotTexture.loadFromFile("web_shot.png"))
            throw std::runtime_error("Failed to load web shot texture.");

        if (!enemyTexture1.loadFromFile("goblin.png") || !enemyTexture2.loadFromFile("venom.png") || !enemyTexture3.loadFromFile("doctor_octopus.png"))
            throw std::runtime_error("Failed to load enemy textures.");

        enemyTexture1.setSmooth(true);
        enemyTexture2.setSmooth(true);
        enemyTexture3.setSmooth(true);

        if (!spidermanTexture.loadFromFile("spiderman.png"))
            throw std::runtime_error("Failed to load Spiderman texture.");

        spiderman = Spiderman(); 

        sf::Vector2f spidermanPosition(150.f, 0.f);
        spiderman.sprite.setTexture(spidermanTexture);
        spiderman.sprite.setPosition(spidermanPosition);

        font.loadFromFile("oswald.ttf");
        if (!font.loadFromFile("oswald.ttf"))
            throw std::runtime_error("Failed to load font.");

        killsText.setFont(font);
        killsText.setCharacterSize(30);
        killsText.setFillColor(sf::Color::White);
        killsText.setPosition(10.f, 10.f);

        gameOverFont.loadFromFile("oswald.ttf");
        if (!gameOverFont.loadFromFile("oswald.ttf"))
            throw std::runtime_error("Failed to load game over font.");

        gameOverText.setFont(gameOverFont);
        gameOverText.setCharacterSize(80);
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setString("GAME OVER");
        gameOverText.setPosition(window.getSize().x / 2.f - gameOverText.getGlobalBounds().width / 2.f,
                                 window.getSize().y / 2.f - gameOverText.getGlobalBounds().height / 2.f);

        backgroundMusic.openFromFile("background_music.ogg");
        if (!backgroundMusic.openFromFile("background_music.ogg"))
            throw std::runtime_error("Failed to load background music.");

        backgroundMusic.setVolume(15);
        backgroundMusic.play();
    }

    // Main function of the game
    void run()
    {
        sf::Clock enemySpawnClock;
        sf::Time enemySpawnTimer;

        while (window.isOpen())
        {
            processEvents();
            update();
            render();
        }
    }

private:
    void processEvents()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
               if (event.key.code == sf::Keyboard::Space && webShots.size() < maxWebShots)
                {
                    sf::Vector2f webShotPosition = spiderman.sprite.getPosition() + sf::Vector2f(spiderman.sprite.getGlobalBounds().width, spiderman.sprite.getGlobalBounds().height / 2.f - webShotTexture.getSize().y / 2.f);
                    WebShot webShot(webShotTexture, webShotPosition);
                    webShots.push_back(webShot);
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && spiderman.sprite.getPosition().y > 0.f)
        {
            spiderman.sprite.move(0.f, -1.f);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && spiderman.sprite.getPosition().y < window.getSize().y - spiderman.sprite.getGlobalBounds().height)
        {
            spiderman.sprite.move(0.f, 1.f);
        }
    }

    void update()
    {
         enemySpawnTimer = enemySpawnClock.getElapsedTime();
        if (enemySpawnTimer.asSeconds() > 0.7f && enemies.size() < maxEnemies)
        {
            int spawnRoll = rand() % 100 + 1;
            if (spawnRoll <= enemySpawnChance)
            {
                sf::Vector2f enemyPosition(window.getSize().x, rand() % static_cast<int>(maxEnemyY - minEnemyY + 1) + minEnemyY);
                sf::Vector2f enemySize(250.f, 250.f); // Modify the size 
                int enemyType = rand() % 3 + 1;
                sf::Texture& enemyTexture = (enemyType == 1) ? enemyTexture1 : ((enemyType == 2) ? enemyTexture2 : enemyTexture3);
                Enemy enemy(enemyTexture, enemyPosition, enemySize);
                enemies.push_back(enemy);
                enemySpawnClock.restart();
            }
        }

        for (size_t i = 0; i < webShots.size(); i++)
        {
            webShots[i].sprite.move(webShots[i].speed, 0.f);
        }

        for (size_t i = 0; i < enemies.size(); i++)
        {
            enemies[i].sprite.move(-enemies[i].speed, 0.f); // Update the enemy position based on its speed
        }

        checkCollisions();
    }

    void checkCollisions()
    {
        bool isGameOver = false; // Flag to indicate if the game is over

        for (size_t i = 0; i < enemies.size(); i++)
        {
            if (spiderman.sprite.getGlobalBounds().intersects(enemies[i].sprite.getGlobalBounds()))
            {
                isGameOver = true;
                break;
            }
        }

        if (isGameOver)
        {
            // Game over logic
            window.draw(gameOverText);
            window.display();
            sf::sleep(sf::seconds(3)); // Pause the game for 3 seconds before closing the window
            window.close();
        }

        for (size_t i = 0; i < webShots.size(); i++)
        {
            for (size_t j = 0; j < enemies.size(); j++)
            {
                if (webShots[i].sprite.getGlobalBounds().intersects(enemies[j].sprite.getGlobalBounds()))
                {
                    kills++;
                    webShots.erase(webShots.begin() + i);
                    enemies.erase(enemies.begin() + j);
                    i--;
                    break;
                }
            }
        }
    }


    void render()
    {
        window.clear();
        window.draw(backgroundSprite);
        window.draw(spiderman.sprite);

        for (size_t i = 0; i < webShots.size(); i++)
        {
            window.draw(webShots[i].sprite);
        }

        for (size_t i = 0; i < enemies.size(); i++)
        {
            window.draw(enemies[i].sprite);
        }

        killsText.setString("Kills: " + std::to_string(kills));
        window.draw(killsText);

        if (kills >= 100)
        {
            window.draw(gameOverText);
        }

        window.display();
    }

// private member variables
private:
    sf::RenderWindow window;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Texture webShotTexture;
    sf::Texture enemyTexture1;
    sf::Texture enemyTexture2;
    sf::Texture enemyTexture3;
    sf::Texture spidermanTexture;
    sf::Font font;
    sf::Font gameOverFont;
    sf::Text killsText;
    sf::Text gameOverText;
    sf::Music backgroundMusic;
    Spiderman spiderman;

    std::vector<WebShot> webShots;
    std::vector<Enemy> enemies;

    const float maxWebShots = 5.f;
    const float enemySpawnChance = 100.f;
    const float maxEnemies = 10.f;
    const float minEnemyY = 50.f;
    const float maxEnemyY = 500.f;
    int kills = 0;

    sf::Clock enemySpawnClock;
    sf::Time enemySpawnTimer;
};

int main()
{
    Game game;
    game.run();

    return 0;
}