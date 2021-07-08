#include "Engine.h"
#include "Game.h"
#include <stdlib.h>
#include <memory.h>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cassert>

constexpr float M_PI = 3.14159265358979323846;
constexpr float ROTATIONSPEED = 3.5;
constexpr auto ACCELERATION = 20;
constexpr auto MAXSPEED = 100;
constexpr auto BULLETSPEED = 150;
constexpr auto BULLETTIME = 3;
constexpr int BULLETSIZE = 3;
constexpr auto NONCREATIONRADIUS = 600;
constexpr auto LIVES = 3;
constexpr auto SIZE = 15;
constexpr float PAUSETIME = 1.0f;

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

float Distance(POINT a, POINT b) {
    return std::min(std::min(std::sqrtf(pow(a.x - b.x, 2) + pow(a.y - b.y, 2)),
        std::sqrtf(pow(a.x + b.x - SCREEN_WIDTH, 2) + pow(a.y - b.y, 2))),
        std::min(std::sqrtf(pow(a.x - b.x, 2) + pow(a.y + b.y - SCREEN_HEIGHT, 2)),
            std::sqrtf(pow(a.x + b.x - SCREEN_WIDTH, 2) + pow(a.y + b.y - SCREEN_HEIGHT, 2))));
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

    for ( int x = x1, y = y1; x != x2 || y != y2; ) {  /* loop */
        buff[mod(y, SCREEN_HEIGHT) * SCREEN_WIDTH + mod(x, SCREEN_WIDTH)] = BGRA({ 255, 255, 255, 0 }).GetInt();
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y += sy; } /* e_xy+e_y < 0 */
    }
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
}

Player::Player() : GameObject() {
    pos = INIT_POS;
    dir = 0;
    speed = {0, 0};
}

void Player::Draw(uint32_t buff[]) {
    // Calculate 4 dots for creating triangle-like player
    POINT d1 = { pos.x + SIZE * cosf(dir), pos.y + SIZE * sinf(dir) };
    POINT d2 = { pos.x + SIZE * cosf(dir + 5 * M_PI / 6), pos.y + SIZE * sinf(dir + 5 * M_PI / 6) };
    POINT d3 = { pos.x + 0.6 * SIZE * cosf(dir + M_PI), pos.y + 0.6 * SIZE * sinf(dir + M_PI) };
    POINT d4 = { pos.x + SIZE * cosf(dir - 5 * M_PI / 6), pos.y + SIZE * sinf(dir - 5 * M_PI / 6) };
    // Call Bresenham's line algorithm 4 times
    Bresenham(buff, d1, d2);
    Bresenham(buff, d2, d3);
    Bresenham(buff, d3, d4);
    Bresenham(buff, d4, d1);
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
}

void Player::Accelerate(float dt) {
    POINT newSpeed = speed;
    newSpeed.x += ACCELERATION * cosf(dir) * dt;
    newSpeed.y += ACCELERATION * sinf(dir) * dt;
    if (std::pow(newSpeed.x, 2) + std::pow(newSpeed.y, 2) <= std::pow(MAXSPEED, 2)) {
        speed = newSpeed;
    }
    return;
}

Bullet::Bullet(Player player) : GameObject() {
    dir = player.GetDirection();
    speed = player.GetSpeed() + BULLETSPEED;
    SetInitPosition(player);
    SetSize(BULLETSIZE);
    SetColor({ 255, 255, 255, 0 }); 
    ttl = BULLETTIME;
}

void Bullet::SetInitPosition(Player player) {
    SetPosition(player.GetPosition());
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

Asteroid::Asteroid(Asteroid prev, bool type) {
    speedType = prev.GetSpeedType();
    assert(prev.GetSizeType() != AsteroidSize::SMALL);
    sizeType = AsteroidSize(static_cast<int>(prev.GetSizeType()) - 1);
    SetInitSize(sizeType);
    SetSpeed(prev.GetSpeed() * sqrtf(2));
    SetDirection(prev.GetDirection() + (type) ? M_PI : -M_PI / 4);
    SetPosition(prev.GetPosition());
    SetInitColor(speedType);
}

Asteroid::Asteroid(AsteroidSpeed argSpeed, AsteroidSize argSize) {
    speedType = argSpeed;
    sizeType = argSize;
    SetInitSize(argSize);
    SetInitSpeed(speedType);
    SetInitDirection();
    SetInitPosition();
    SetInitColor(argSpeed);
}

void Asteroid::SetInitSize(AsteroidSize argSize) {
    switch (argSize) {
    case AsteroidSize::SMALL:
        SetSize(10.0);
        break;
    case AsteroidSize::NORMAL:
        SetSize(20.0);
        break;
    case AsteroidSize::BIG:
        SetSize(30.0);
        break;
    }
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
}
void Asteroid::SetInitDirection() {
    SetDirection(static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX) * 2 * M_PI);
    return;
}
void Asteroid::SetInitPosition() {
    POINT argPos;
    do {
        argPos = { static_cast <float> (std::rand() % SCREEN_WIDTH), static_cast <float> (std::rand() % SCREEN_WIDTH) };
    } while (Distance(argPos, INIT_POS) < NONCREATIONRADIUS);
    SetPosition(argPos);
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
    levelDifficulties = { {10, 10, 10}, {4, 1, 1}, {2, 2, 2}, {0, 2, 4} };
    lifes = 3;
    level = 0;
    points = 0;
    state = GameState::GAME;
}

void GameManager::StartLevel() {
    assert(level >= 0 && level <= 3);
    for (int i = 0; i < levelDifficulties[level].size(); i++) {
        for (int j = 0; j < levelDifficulties[level][i]; j++) {
            asteroids.push_back(Asteroid(static_cast<AsteroidSpeed>(i), AsteroidSize::BIG));
        }
    }
}

void GameManager::UpdateTime(float dt) {
    if (time != 0) {
        time = (time - dt <= 0) ? 0.0f : time - dt;
    }
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
    //for (auto itb = bullets.begin(); itb != bullets.end();) {
    //    itb->Move(dt);
    //    for (auto ita = asteroids.begin(); ita != asteroids.end();) {
    //        if (Distance(ita->GetPosition(), itb->GetPosition()) <= ita->GetSize() + itb->GetSize()) {
    //            itb = bullets.erase(itb);
    //            if (ita->GetSizeType() != AsteroidSize::SMALL) {
    //                asteroids.push_back(Asteroid(*ita, false));
    //                asteroids.push_back(Asteroid(*ita, true));
    //            }
    //            ita = asteroids.erase(ita);
    //        }
    //        else {
    //            ita++;
    //        }
    //    }
    //}
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
}

//
//  IDEAS:
//  Collision detection (if asteroids - change direction [probably don't implement it], if player - take the life and don't change the direction)
//  Create default BG
//  If player dies in N first seconds after respawn - no penalties
//  When asteroid breaks into 2 part - tangential speed remains the same and normal speed are create for purpose of 30 degrees for initial derection
//  Start New level after LevelOver() == true
//  MAIN MENU with LEADERBOARD (stored in file as binary data)
//

//
//  EXPLANATIONS:
//  get_cursor_x(), get_cursor_y() - get mouse cursor position                                                 - WE DON'T NEED IT
//  is_mouse_button_pressed(int button) - check if mouse button is pressed (0 - left button, 1 - right button) - WE DON'T NEED IT
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
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt) {
    gameManager.UpdateTime(dt);
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

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw() {
    // clear backbuffer
    memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
    for (auto& x : gameManager.bullets) {
        x.Draw(reinterpret_cast<uint32_t*>(buffer));
    }
    for (auto& x : gameManager.asteroids) {
        x.Draw(reinterpret_cast<uint32_t*>(buffer));
    }
    gameManager.player.Draw(reinterpret_cast<uint32_t*>(buffer));
}

// free game data in this function
void finalize() {
}
