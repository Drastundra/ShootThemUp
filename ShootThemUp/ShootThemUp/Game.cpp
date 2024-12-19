#include "Game.h"
#include <algorithm>
#include <SFML/Audio.hpp>



Game::Game()
    : mWindow(sf::VideoMode(1500, 1000), "Asteroid Survival"),
    mIsPlaying(false),
    mIsGameOver(false),
    mIsEnteringName(false) {
    mRNG.seed(std::chrono::steady_clock::now().time_since_epoch().count());
}

void Game::run() {
    sf::Clock clock;

    sf::Music music;
    if (!music.openFromFile("C:/Users/thoma/OneDrive/Documents/GitHub/projet_shoot_em_up/cours3decembre2/pvzgw2 ops special wave (version b).mp3"))
    { }
    music.play();
    while (mWindow.isOpen()) {
        music.setLoop(true);
        processEvents();
        sf::Time deltaTime = clock.restart();
        if (mIsPlaying) {
            update(deltaTime);
        }
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (mWindow.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            mWindow.close();
        }

        if (!mIsPlaying && !mIsEnteringName) {
            processMenuEvents(event);
        }
        else if (mIsEnteringName) {
            processNameEntryEvents(event);
        }
        else {
            processGameplayEvents(event);
        }
    }

    if (mIsPlaying && sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && mPlayer.canShoot()) {
        sf::Vector2f playerPos = mPlayer.getPosition();
        float playerRotation = mPlayer.getRotation();
        mProjectiles.emplace_back(playerPos, playerRotation);
        mPlayer.resetShootClock();
    }
}

void Game::processMenuEvents(const sf::Event& event) {
    mMenu.handleInput(event, mIsPlaying);
}

void Game::processGameplayEvents(const sf::Event& event) {
    mPlayer.handleInput(event);
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Space &&
        mPlayer.canShoot()) {
        // Cr�ation d'un nouveau projectile
        sf::Vector2f playerPos = mPlayer.getPosition();
        float playerRotation = mPlayer.getRotation();
        mProjectiles.emplace_back(playerPos, playerRotation);
        mPlayer.resetShootClock();
    }
}

void Game::spawnAsteroid() {
    // Choisir un c�t� al�atoire (0: haut, 1: droite, 2: bas, 3: gauche)
    int side = randomInt(0, 3);
    sf::Vector2f position;
    sf::Vector2f velocity;

    switch (side) {
    case 0: // Haut
        position = sf::Vector2f(randomFloat(0.f, 1500.f), -50.f);
        velocity = sf::Vector2f(randomFloat(-200.f, 200.f), randomFloat(100.f, 300.f));
        break;
    case 1: // Droite
        position = sf::Vector2f(1550.f, randomFloat(0.f, 1000.f));
        velocity = sf::Vector2f(randomFloat(-300.f, -100.f), randomFloat(-200.f, 200.f));
        break;
    case 2: // Bas
        position = sf::Vector2f(randomFloat(0.f, 1500.f), 1050.f);
        velocity = sf::Vector2f(randomFloat(-200.f, 200.f), randomFloat(-300.f, -100.f));
        break;
    case 3: // Gauche
        position = sf::Vector2f(-50.f, randomFloat(0.f, 1000.f));
        velocity = sf::Vector2f(randomFloat(100.f, 300.f), randomFloat(-200.f, 200.f));
        break;
    }

    mAsteroids.emplace_back(position, velocity);
}

void Game::spawnEnemyShip() {
    // Logique similaire au spawn d'ast�ro�de
    int side = randomInt(0, 3);
    sf::Vector2f position;
    sf::Vector2f velocity;

    switch (side) {
    case 0: // Haut
        position = sf::Vector2f(randomFloat(0.f, 1500.f), -50.f);
        velocity = sf::Vector2f(randomFloat(-100.f, 100.f), randomFloat(50.f, 150.f));
        break;
        // ... autres cas similaires � spawnAsteroid
    }

    mEnemyShips.emplace_back(position, velocity);
}


void Game::checkCollisions() {
    // V�rification des collisions projectiles-ast�ro�des
    for (auto projectileIt = mProjectiles.begin(); projectileIt != mProjectiles.end();) {
        bool projectileHit = false;

        for (auto asteroidIt = mAsteroids.begin(); asteroidIt != mAsteroids.end();) {
            if (projectileIt->getBounds().intersects(asteroidIt->getBounds())) {
                asteroidIt = mAsteroids.erase(asteroidIt);
                projectileHit = true;
                break;
            }
            else {
                ++asteroidIt;
            }
        }

        if (projectileHit) {
            projectileIt = mProjectiles.erase(projectileIt);
        }
        else {
            ++projectileIt;
        }
    }

    // V�rification des collisions joueur-ast�ro�des
    for (const auto& asteroid : mAsteroids) {
        if (mPlayer.getBounds().intersects(asteroid.getBounds())) {
            mIsPlaying = false;
            mIsGameOver = true;
            return;
        }
    }
    for (auto enemyIt = mEnemyShips.begin(); enemyIt != mEnemyShips.end();) {
        bool enemyHit = false;

        for (auto projectileIt = mProjectiles.begin(); projectileIt != mProjectiles.end();) {
            if (projectileIt->getBounds().intersects(enemyIt->getBounds())) {
                enemyIt = mEnemyShips.erase(enemyIt);
                projectileIt = mProjectiles.erase(projectileIt);
                enemyHit = true;
                break;
            }
            else {
                ++projectileIt;
            }
        }

        if (!enemyHit) {
            ++enemyIt;
        }
    }

    for (const auto& projectile : mEnemyProjectiles) {
        if (projectile.getBounds().intersects(mPlayer.getBounds())) {
            mIsPlaying = false;
            mIsGameOver = true;
            return;
        }
    }
}

void Game::processNameEntryEvents(const sf::Event& event) {
    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == '\b' && !mPlayerName.empty()) {
            mPlayerName.pop_back();
        }
        else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
            mPlayerName += static_cast<char>(event.text.unicode);
        }
    }
}

void Game::update(sf::Time deltaTime) {
    mPlayer.update(deltaTime);
    for (auto& projectile : mProjectiles) {
        projectile.update(deltaTime);
    }

    for (auto& asteroid : mAsteroids) {
        asteroid.update(deltaTime);
    }

    for (auto& enemy : mEnemyShips) {
        enemy.update(deltaTime, mPlayer.getPosition());
        auto newProjectiles = enemy.shoot(mPlayer.getPosition());
        mEnemyProjectiles.insert(mEnemyProjectiles.end(),
            newProjectiles.begin(),
            newProjectiles.end());
    }

    for (auto& projectile : mEnemyProjectiles) {
        projectile.update(deltaTime);
    }



    if (mEnemySpawnClock.getElapsedTime().asSeconds() >= 5.0f) {
        spawnEnemyShip();
        mEnemySpawnClock.restart();
    }

    if (mAsteroidSpawnClock.getElapsedTime().asSeconds() >= 2.0f) {
        spawnAsteroid();
        mAsteroidSpawnClock.restart();
    }

    mProjectiles.erase(
        std::remove_if(mProjectiles.begin(), mProjectiles.end(),
            [](const Projectile& p) { return p.isOffscreen(); }),
        mProjectiles.end());

    mAsteroids.erase(
        std::remove_if(mAsteroids.begin(), mAsteroids.end(),
            [](const Asteroid& a) { return a.isOffscreen(); }),
        mAsteroids.end());

    mEnemyShips.erase(
        std::remove_if(mEnemyShips.begin(), mEnemyShips.end(),
            [](const EnemyShip& e) { return e.isOffscreen(); }),
        mEnemyShips.end());

    // Nettoyage des projectiles ennemis hors �cran
    mEnemyProjectiles.erase(
        std::remove_if(mEnemyProjectiles.begin(), mEnemyProjectiles.end(),
            [](const Projectile& p) { return p.isOffscreen(); }),
        mEnemyProjectiles.end());

    // V�rification des collisions
    checkCollisions();

}

void Game::render() {
    mWindow.clear();

    if (!mIsPlaying) {
        if (mIsGameOver) {
            // Code de Game Over
        }
        else {
            mMenu.draw(mWindow);
        }
    }
    else {
        mPlayer.draw(mWindow);

        // Dessin des projectiles
        for (const auto& projectile : mProjectiles) {
            projectile.draw(mWindow);
        }

        // Dessin des ast�ro�des
        for (const auto& asteroid : mAsteroids) {
            asteroid.draw(mWindow);

            for (const auto& enemy : mEnemyShips) {
                enemy.draw(mWindow);
            }
            for (const auto& projectile : mEnemyProjectiles) {
                projectile.draw(mWindow);
            }
        }
    }

    mWindow.display();
}