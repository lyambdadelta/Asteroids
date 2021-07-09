#include "Engine.h"
#include "Game.h"
#include "Bitmap.h"
#include <stdlib.h>
#include <memory.h>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cassert>

constexpr float M_PI = 3.14159265358979323846;
constexpr float ROTATIONSPEED = 1.5;
constexpr auto ACCELERATION = 50;
constexpr auto MAXSPEED = 120;
constexpr auto BULLETSPEED = 200;
constexpr auto BULLETTIME = 3;
constexpr int BULLETSIZE = 3;
constexpr auto NONCREATIONRADIUS = 300;
constexpr auto LIVES = 3;
constexpr auto SIZE = 15;
constexpr float PAUSETIME = 1.0f;
constexpr float INVINCIBLETIME = 5.0f;

uint32_t BGRA::GetInt() const {
    return alpha << 24 | red << 16 | green << 8 | blue;
}
BGRA::BGRA() {
    blue = 0;
    green = 0;
    red = 0;
    alpha = 0;
}
BGRA::BGRA(uint8_t b, uint8_t g, uint8_t r, uint8_t a) {
    blue = b;
    green = g;
    red = r;
    alpha = a;
}

void DrawString(uint32_t buff[], std::string str, uint32_t posx, uint32_t posy, uint32_t size = 4) {
    assert(posx < SCREEN_WIDTH - 4 && posy < SCREEN_HEIGHT - 8);
    uint32_t start;
    for (const auto& x : str) {
        if (bitmap.find(x) == bitmap.end()) {
            continue;
        }
        start = posx;
        for (int j = 0; j < bitmap[x].size(); j++) {
            for (int i = 0; i < bitmap[x][j].size(); i++) {
                if (bitmap[x][j][i]) {
                    for (int k = 0; k < size; k++) {
                        for (int l = 0; l < size; l++) {
                            buff[mod(posy + j * size + k, SCREEN_HEIGHT) * SCREEN_WIDTH + mod(posx + i * size + l, SCREEN_WIDTH)] = BGRA({ 255, 255, 255, 0 }).GetInt();
                        }
                    }
                }
            }
        }
        posx = start + size * (bitmap[x][0].size() + 1);
    }

}

float Distance(POINT a, POINT b) {
    float minValue = std::min({
        powf(a.x - b.x, 2) + powf(a.y - b.y, 2),
        powf(a.x - b.x + SCREEN_WIDTH, 2) + powf(a.y - b.y, 2),
        powf(-a.x + b.x + SCREEN_WIDTH, 2) + powf(a.y - b.y, 2),
        powf(a.x - b.x, 2) + powf(a.y - b.y + SCREEN_HEIGHT, 2),
        powf(a.x - b.x, 2) + powf(-a.y + b.y + SCREEN_HEIGHT, 2),
        powf(a.x - b.x + SCREEN_WIDTH, 2) + powf(a.y - b.y + SCREEN_HEIGHT, 2),
        powf(a.x - b.x + SCREEN_WIDTH, 2) + powf(-a.y + b.y + SCREEN_HEIGHT, 2),
        powf(-a.x + b.x + SCREEN_WIDTH, 2) + powf(a.y - b.y + SCREEN_HEIGHT, 2),
        powf(-a.x + b.x + SCREEN_WIDTH, 2) + powf(-a.y + b.y + SCREEN_HEIGHT, 2),
    });
    return std::sqrtf(minValue);
}

void DecreaseTime(float& t, float dt) {
    if (t != 0) {
        t = (t - dt <= 0) ? 0.0f : t - dt;
    }
}

int mod(int value, int m) {
    value %= m;
    return (value >= 0) ? value : value + m;
}

void Bresenham(uint32_t buff[], POINT d1, POINT d2) {
    int x1 = static_cast<int>(d1.x), x2 = static_cast<int>(d2.x), y1 = static_cast<int>(d1.y), y2 = static_cast<int>(d2.y);
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int e2 = 0;

    for ( int x = x1, y = y1; x != x2 || y != y2; ) {
        buff[mod(y, SCREEN_HEIGHT) * SCREEN_WIDTH + mod(x, SCREEN_WIDTH)] = BGRA({ 255, 255, 255, 0 }).GetInt();
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x += sx; }
        if (e2 <= dx) { err += dx; y += sy; }
    }

    return;
}

static POINT INIT_POS = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

GameObject::GameObject() {
    SetDirection(0.0);
    SetSize(0.0);
    SetSpeed(0.0);
    SetColor({ 0, 0, 0, 0 });
}

float GameObject::GetSize() const {
    return size;
}

float GameObject::GetSpeed() const {
    return speed;
}

float GameObject::GetDirection() const {
    return dir;
}

POINT GameObject::GetPosition() const {
    return pos;
}

uint32_t GameObject::GetColor() const {
    return color.GetInt();
}

void GameObject::Rotate(float angle) {
    dir += angle;
    dir = fmod(dir, 2 * M_PI);
    return;
}

void GameObject::SetColor(BGRA argColor) {
    color = argColor;
    return;
}

void GameObject::SetDirection(float argDir) {
    dir = argDir;
    return;
}

void GameObject::SetSize(float argSize) {
    size = argSize;
    return;
}

void GameObject::SetSpeed(float argSpeed) {
    speed = argSpeed;
    return;
}

void GameObject::SetPosition(POINT argPosition) {
    pos = argPosition;
    return;
}

void GameObject::Draw(uint32_t buff[]) {
    int x = static_cast<int>(pos.x);
    int y = static_cast<int>(pos.y);
    int R = static_cast<int>(size);

    for (int i = x - R; i <= x + R; i++) {
        for (int j = y - R; j <= y + R; j++) {
            if (std::pow(x - i, 2) + std::pow(y - j, 2) <= std::pow(R, 2)) {
                buff[mod(j, SCREEN_HEIGHT) * SCREEN_WIDTH + mod(i, SCREEN_WIDTH)] = GetColor();
            }
        }
    }
    return;
}

void GameObject::Move(float dt) {
    pos.x = fmod(pos.x + speed * cosf(dir) * dt, SCREEN_WIDTH);
    if (pos.x < 0) {
        pos.x += SCREEN_WIDTH;
    }
    pos.y = fmod(pos.y + speed * sinf(dir) * dt, SCREEN_HEIGHT);
    if (pos.x < 0) {
        pos.x += SCREEN_HEIGHT;
    }
    return;
}

Player::Player() : GameObject() {
    pos = INIT_POS;
    dir = 0;
    size = SIZE;
    speed = {0, 0};
    return;
}

void Player::Draw(uint32_t buff[]) {
    // Calculate 4 dots for creating triangle-like player
    POINT d1 = { pos.x + size * cosf(dir), pos.y + size * sinf(dir) };
    POINT d2 = { pos.x + size * cosf(dir + 5 * M_PI / 6), pos.y + size * sinf(dir + 5 * M_PI / 6) };
    POINT d3 = { pos.x + 0.6 * size * cosf(dir + M_PI), pos.y + 0.6 * size * sinf(dir + M_PI) };
    POINT d4 = { pos.x + size * cosf(dir - 5 * M_PI / 6), pos.y + size * sinf(dir - 5 * M_PI / 6) };
    // Call Bresenham's line algorithm 4 times
    Bresenham(buff, d1, d2);
    Bresenham(buff, d2, d3);
    Bresenham(buff, d3, d4);
    Bresenham(buff, d4, d1);
    return;
}

void Player::Move(float dt) {
    pos.x = fmod(pos.x + speed.x * dt, SCREEN_WIDTH);
    if (pos.x < 0) {
        pos.x += SCREEN_WIDTH;
    }
    pos.y = fmod(pos.y + speed.y * dt, SCREEN_HEIGHT);
    if (pos.x < 0) {
        pos.x += SCREEN_HEIGHT;
    }
    return;
}

void Player::Accelerate(float dt) {
    POINT newSpeed = speed;
    newSpeed.x += ACCELERATION * cosf(dir) * dt;
    newSpeed.y += ACCELERATION * sinf(dir) * dt;
    float newSpeedMod = std::sqrtf(std::pow(newSpeed.x, 2) + std::pow(newSpeed.y, 2));
    if (newSpeedMod > MAXSPEED) {
        speed.x = newSpeed.x / newSpeedMod * MAXSPEED;
        speed.y = newSpeed.y / newSpeedMod * MAXSPEED;
    }
    else {
        speed = newSpeed;
    }
    return;
}

Bullet::Bullet(Player player) : GameObject() {
    SetDirection(GetDirection());
    SetSpeed(BULLETSPEED);
    SetInitPosition(player);
    SetSize(BULLETSIZE);
    SetColor({ 255, 255, 255, 0 }); 
    ttl = BULLETTIME;
    return;
}

void Bullet::SetInitPosition(Player player) {
    SetPosition({ player.GetPosition().x + (player.GetSize() + GetSize()) * cosf(player.GetDirection()),  
                    player.GetPosition().y + (player.GetSize() + GetSize()) * sinf(player.GetDirection()) });
    return;
}

bool Bullet::UpdateTime(float dt) {
    ttl -= dt;
    if (ttl <= 0) {
        ttl = 0;
        return true;
    }
    return false;
}

AsteroidSpeed Asteroid::GetSpeedType() const {
    return speedType;
}

AsteroidSize Asteroid::GetSizeType() const {
    return sizeType;
}

Asteroid::Asteroid(const Asteroid& prev, bool type) {
    speedType = prev.GetSpeedType();
    assert(prev.GetSizeType() != AsteroidSize::SMALL);
    sizeType = AsteroidSize(static_cast<int>(prev.GetSizeType()) - 1);
    SetInitSize(sizeType);
    SetSpeed(prev.GetSpeed() * 2 / sqrtf(3));
    SetDirection(prev.GetDirection() + ((type) ? M_PI : -M_PI) / 6);
    SetPosition(prev.GetPosition());
    SetInitColor(speedType);
    return;
}

Asteroid::Asteroid(AsteroidSpeed argSpeed, AsteroidSize argSize) {
    speedType = argSpeed;
    sizeType = argSize;
    SetInitSize(argSize);
    SetInitSpeed(speedType);
    SetInitDirection();
    SetInitPosition();
    SetInitColor(argSpeed);
    return;
}

void Asteroid::SetInitSize(AsteroidSize argSize) {
    switch (argSize) {
    case AsteroidSize::SMALL:
        SetSize(20.0);
        break;
    case AsteroidSize::NORMAL:
        SetSize(27.0);
        break;
    case AsteroidSize::BIG:
        SetSize(35.0);
        break;
    }
    return;
}

void Asteroid::SetInitSpeed(AsteroidSpeed argSpeed) {
    switch (argSpeed) {
    case AsteroidSpeed::SLOW:
        SetSpeed(40);
        break;
    case AsteroidSpeed::MEDIUM:
        SetSpeed(60);
        break;
    case AsteroidSpeed::FAST:
        SetSpeed(100);
        break;
    }
    return;
}
void Asteroid::SetInitDirection() {
    SetDirection(static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX) * 2 * M_PI);
    return;
}

void Asteroid::SetInitPosition() {
    POINT argPos;
    do {
        argPos = { static_cast <float> (std::rand() % SCREEN_WIDTH), static_cast <float> (std::rand() % SCREEN_HEIGHT) };
    } while (Distance(argPos, INIT_POS) < NONCREATIONRADIUS);
    SetPosition(argPos);
    return;
}

void Asteroid::SetInitColor(AsteroidSpeed argSpeed) {
    switch (argSpeed) {
    case AsteroidSpeed::SLOW:
        SetColor({ 0, 255, 0, 0 });
        break;
    case AsteroidSpeed::MEDIUM:
        SetColor({ 255, 0, 0, 0 });
        break;
    case AsteroidSpeed::FAST:
        SetColor({ 0, 0, 255, 0 });
        break;
    }
    speedType = argSpeed;
    return;
}

GameManager::GameManager() {
    levelDifficulties = { {5, 1, 0}, {3, 2, 1}, {1, 3, 2}, {1, 1, 4} };
    lifes = LIVES;
    level = 0;
    invincibleTime = INVINCIBLETIME;
    time = 0;
    points = maxPoints = 0;
    state = GameState::GAME;
    return;
}

void GameManager::RestartGame() {
    lifes = LIVES;
    points = 0;
    level = 0;
    time = 0;
    state = GameState::GAME;
    StartLevel();
    return;
}

void GameManager::StartLevel() {
    for (int i = 0; i < levelDifficulties[level].size(); i++) {
        for (int j = 0; j < levelDifficulties[level][i]; j++) {
            asteroids.push_back(Asteroid(static_cast<AsteroidSpeed>(i), AsteroidSize::BIG));
        }
    }
    return;
}

void GameManager::NextLevel() {
    assert(asteroids.empty());
    if (level != levelDifficulties.size() - 1) {
        level++;
    }
    else {
        // add 1 Fast every time after beating last level 
        levelDifficulties[levelDifficulties.size() - 1][2]++;
    }
    time = 0;
    bullets.clear();
    player = Player();
    StartLevel();
    return;
}

void GameManager::setState(GameState argState) {
    state = argState;
}

GameState GameManager::GetState() const {
    return state;
}

void GameManager::GameOver() {
    asteroids.clear();
    bullets.clear();
    player.SetColor({ 0, 0, 0, 0 });
    if (points > maxPoints) {
        maxPoints = points;
    }
    setState(GameState::GAMEOVER);
}

void GameManager::UpdateTimeGame(float dt) {
    DecreaseTime(time, dt);
    DecreaseTime(invincibleTime, dt);
    for (auto it = bullets.begin(); it != bullets.end();) {
        it = (it->UpdateTime(dt)) ? bullets.erase(it) : ++it;
    }
    player.Move(dt);
    for (auto& x : asteroids) {
        x.Move(dt);
    }
    for (auto& x : bullets) {
        x.Move(dt);
    }
    // Collision between Player and Asteroids
    for (const auto& x : asteroids) {
        if (Distance(x.GetPosition(), player.GetPosition()) <= x.GetSize() + player.GetSize() * 0.6) {
            player = Player();
            if (invincibleTime == 0) {
                lifes--;
            }
            invincibleTime = INVINCIBLETIME;
        }
    }
    // Collision between Player and bullets
    for (auto itB = bullets.begin(); itB != bullets.end(); itB++) {
        if (Distance(itB->GetPosition(), player.GetPosition()) <= itB->GetSize() + player.GetSize() * 0.6) {
            bullets.erase(itB);
            player = Player();
            lifes--;
            invincibleTime = INVINCIBLETIME;
            break;
        }
    }
    // Collision between Bullets and Asteroids
    for (auto itB = bullets.begin(); itB != bullets.end();) {
        bool next = true;
        for (auto itA = asteroids.begin(); itA != asteroids.end(); itA++) {
            if (Distance(itA->GetPosition(), itB->GetPosition()) <= itA->GetSize() + itB->GetSize()) {
                auto t = Distance(itA->GetPosition(), itB->GetPosition());
                next = false;
                Asteroid parent = *itA;
                itA = asteroids.erase(itA);
                itB = bullets.erase(itB);
                if (parent.GetSizeType() != AsteroidSize::SMALL) {
                    asteroids.push_back(Asteroid(parent, false));
                    asteroids.push_back(Asteroid(parent, true));
                }
                points += (3 - static_cast<int64_t>(parent.GetSizeType())) * (pow(10, static_cast<int>(parent.GetSpeedType()))) * (level + 1);
                break;
            }
        }
        if (next) {
            itB++;
        }
    }
    if (IsLevelOver()) {
        NextLevel();
        return;
    }
    if (IsGameOver()) {
        GameOver();
        return;
    }
    return;
}

bool GameManager::IsLevelOver() const {
    return asteroids.empty();
}

int GameManager::GetLifes() const {
    return lifes;
}

void GameManager::LostLife() {
    lifes--;
    return;
}

bool GameManager::IsGameOver() const {
    return !lifes;
}

bool GameManager::CanShoot() const {
    return time == 0;
}

void GameManager::Shoot() {
    time = PAUSETIME;
    bullets.push_back(Bullet(player));
    return;
}

uint64_t GameManager::GetPoints() const {
    return points;
}

uint64_t GameManager::GetMaxPoints() const {
    return maxPoints;
}

//
//  IDEAS:
//  Collision detection (if asteroids - change direction [probably don't implement it])
//  Create default BG
//  MAIN MENU with LEADERBOARD (stored in file as binary data)
//  Add multiplayer and display lives/point for both players
//

//
//  EXPLANATIONS:
//  get_cursor_x(), get_cursor_y() - get mouse cursor position                                                 - WE CAN USE IT FOR MAIN MENU
//  is_mouse_button_pressed(int button) - check if mouse button is pressed (0 - left button, 1 - right button) - WE CAN USE IT FOR MAIN MENU
//  clear_buffer() - set all pixels in buffer to 'black'
//  is_window_active() - returns true if window is active
//  schedule_quit_game() - quit game after act()
//

static GameManager gameManager;

// initialize game data in this function
void initialize() {
    srand(static_cast<unsigned int>(time(0)));
    gameManager = {};
    gameManager.StartLevel();
    return;
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt) {
    if (is_window_active()) {
        if (gameManager.GetState() == GameState::GAME) {
            gameManager.UpdateTimeGame(dt);
            if (is_key_pressed(VK_ESCAPE)) {
                schedule_quit_game();
            }
            if (is_key_pressed(VK_LEFT)) {
                gameManager.player.Rotate(-dt * ROTATIONSPEED);
            }
            if (is_key_pressed(VK_RIGHT)) {
                gameManager.player.Rotate(dt * ROTATIONSPEED);
            }
            if (is_key_pressed(VK_UP)) {
                gameManager.player.Accelerate(dt);
            }
            if (is_key_pressed(VK_SPACE)) {
                if (gameManager.CanShoot()) {
                    gameManager.Shoot();
                }
            }
        }
        else if (gameManager.GetState() == GameState::GAMEOVER) {
            if (is_key_pressed(VK_ESCAPE)) {
                schedule_quit_game();
            }
            if (is_key_pressed('F')) {
                gameManager.RestartGame();
            }
        }
    }
    return;
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw() {
    // clear backbuffer
    memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
    if (gameManager.GetState() == GameState::GAME || gameManager.GetState() == GameState::PAUSE) {
        for (auto& x : gameManager.bullets) {
            x.Draw(reinterpret_cast<uint32_t*>(buffer));
        }
        for (auto& x : gameManager.asteroids) {
            x.Draw(reinterpret_cast<uint32_t*>(buffer));
        }
        gameManager.player.Draw(reinterpret_cast<uint32_t*>(buffer));
        DrawString(reinterpret_cast<uint32_t*>(buffer), "score: " + std::to_string(gameManager.GetPoints()), 10, 10);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "highscore: " + std::to_string(gameManager.GetMaxPoints()), 400, 10);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "lives: " + std::to_string(gameManager.GetLifes()), SCREEN_WIDTH - 140, 10);
        return;
    }
    else if (gameManager.GetState() == GameState::GAMEOVER) {
        DrawString(reinterpret_cast<uint32_t*>(buffer), "game over!", 300, SCREEN_HEIGHT/2 - 50, 10);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "your score: " + std::to_string(gameManager.GetPoints()), 300, SCREEN_HEIGHT / 2 + 50);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "your highscore: " + std::to_string(gameManager.GetMaxPoints()), 300, SCREEN_HEIGHT / 2 + 100);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "press f to pay replay! ", 300, SCREEN_HEIGHT / 2 + 150);
    }
}

// free game data in this function
void finalize() {
    return;
}
