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
constexpr auto ACCELERATION = 1;
constexpr auto MAXSPEED = 3;
constexpr auto BULLETSPEED = 4;
constexpr auto BULLETTIME = 5;
constexpr auto NONCREATIONRADIUS = 0;
constexpr auto LIVES = 3;

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

Player::Player() : GameObject() {
    pos = INIT_POS;
    dir = 0;
    speed = 0;
}

Bullet::Bullet(Player player) : GameObject() {
    dir = player.GetDirection();
    speed = player.GetSpeed() + BULLETSPEED;
    SetInitPosition(player);
    ttl = BULLETTIME;
}

void Bullet::SetInitPosition(Player player) {
    SetPosition(player.GetPosition());
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

void Asteroid::Draw(uint32_t buff[]) {
    int x = static_cast<int>(pos.x);
    int y = static_cast<int>(pos.y);
    int R = static_cast<int>(size);
    for (int i = x - R; i <= x + R; i++) {
        for (int j = y - R; j <= y + R; j++) {
            if (std::pow(x - i, 2) + std::pow(y - j, 2) <= std::pow(R, 2)) {
                buff[mod(j, SCREEN_HEIGHT) * SCREEN_WIDTH + mod(i, SCREEN_WIDTH)] = color.GetInt();
            }
        }
    }
}

void Asteroid::Move(float dt) {
    pos.x = fmod(pos.x - speed * sinf(dir) * dt, SCREEN_WIDTH);
    pos.y = fmod(pos.y + speed * cosf(dir) * dt, SCREEN_WIDTH);
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
}

GameManager::GameManager() {
    levelDifficulties = { {6, 0, 0}, {4, 1, 1}, {2, 2, 2}, {0, 2, 4} };
    lifes = 3;
    level = 0;
    points = 0;
}

void GameManager::StartLevel() {
    assert(level >= 0 && level <= 3);
    for (int i = 0; i < levelDifficulties[level].size(); i++) {
        for (int j = 0; j < levelDifficulties[level][i]; j++) {
            asteroids.push_back(Asteroid(static_cast<AsteroidSpeed>(i), AsteroidSize::BIG));
        }
    }
}

bool GameManager::IsLevelOver() {
    return asteroids.empty();
}

int GameManager::GetLifes() {
    return lifes;
}
void GameManager::LostLife() {
    lifes--;
}
bool GameManager::IsGameOver() {
    return !lifes;
}

//
//  IDEAS:
//  Vector for all objects, projecting their possition on the buffer
//  Each object is a class derieved from the parent-object class with the sizes
//  For bullets TTL (time to live)
//  Collision detection (if asteroids - change direction [probably don't implement it], if player - take the life and don't change the direction)
//  Different asteroid types (difficulties), different colors and speed (maybe diffirent recurse types)
//  Create default BG
//  From creation position at the start at radius R no asteroids are spawned
//  If player dies in N first seconds after respawn - no penalties
//  When asteroid breaks into 2 part - tangential speed remains the same and normal speed are create for purpose of 30 degrees for initial derection
//  Few levels with increasing number of faster asteroids (probably max level = all M-type asteroids)
//  MAIN MENU with LEADERBOARD (stored in file as binary data)
//

//
//  EXPLANATIONS:
//  BGRA32 - blue, green, red, alpha - 8 bit for each
//  VK_LEFT/VK_RIGHT - rotation, VK_UP - acceleration, VK_SPACE - shooting
//  is_key_pressed(int button_vk_code) - check if a key is pressed,
//                                       use keycodes (VK_SPACE, VK_RIGHT, VK_LEFT, VK_UP, VK_DOWN, 'A', 'B')
//
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
        // Acceseleration
    }
    if (is_key_pressed(VK_SPACE)) {
        // Shoot
    }
    for (auto& x : gameManager.asteroids) {
        x.Move(dt);
    }
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw() {
    // clear backbuffer
    memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
    for (auto& x : gameManager.asteroids) {
        x.Draw(reinterpret_cast<uint32_t*>(buffer));
    }
}

// free game data in this function
void finalize() {
}
