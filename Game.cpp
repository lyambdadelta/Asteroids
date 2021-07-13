#include "Engine.h"
#include "Game.h"
#include "Bitmap.h"
#include <fstream>
#include <stdlib.h>
#include <memory.h>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cassert>

constexpr float M_PI = 3.14159265358979323846;
constexpr float ROTATIONSPEED = 2.0;
constexpr auto ACCELERATION = 50;
constexpr auto MAXSPEED = 120;
constexpr auto BULLETSPEED = 200;
constexpr auto BULLETTIME = 3;
constexpr int BULLETSIZE = 3;
constexpr auto NONCREATIONRADIUS = 300;
constexpr auto LIVES = 3;
constexpr auto SIZE = 15;
constexpr float PAUSETIME = 0.7f;
constexpr float INVINCIBLETIME = 3.0f;
static POINT INIT_POS = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
static POINT INIT_POS1 = { SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2 };
static POINT INIT_POS2 = { 2 * SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2 };

uint32_t defaultBG[SCREEN_HEIGHT][SCREEN_WIDTH];

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

void Bresenham(uint32_t buff[], POINT d1, POINT d2, uint32_t color) {
    int x1 = static_cast<int>(d1.x), x2 = static_cast<int>(d2.x), y1 = static_cast<int>(d1.y), y2 = static_cast<int>(d2.y);
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int e2 = 0;

    for ( int x = x1, y = y1; x != x2 || y != y2; ) {
        buff[mod(y, SCREEN_HEIGHT) * SCREEN_WIDTH + mod(x, SCREEN_WIDTH)] = color;
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x += sx; }
        if (e2 <= dx) { err += dx; y += sy; }
    }

    return;
}

//float CalculateDirection(POINT a, POINT b) {
//    return atan2f(a.x - b.x, a.y - b.y);
//}

//POINT CalculateSpeed(float initDir, float speed, float collisionDir) {
//    return { speed * cosf(collisionDir - initDir), speed * sinf(collisionDir - initDir) };
//}

bool LoadDefaultBG(uint32_t buff[], std::string name) {
    std::ifstream input(name);
    if (input.is_open()) {
        for (int i = 9; i < SCREEN_HEIGHT - 9; i++) {
            for (int j = 12; j < SCREEN_WIDTH - 12; j++) {
                input >> buff[i * SCREEN_WIDTH + j];
            }
        }
        input.close();
        return true;
    }
    return false;
}

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

Player::Player() {}

Player::Player(GameType argType, bool first=true) {
    pos = (argType == GameType::SIGLEPLAYER) ? INIT_POS : (first) ? INIT_POS2 : INIT_POS1;
    dir = - M_PI / 2;
    size = SIZE;
    speed = {0, 0};
    (argType == GameType::MULTIPLAYER) ? (first) ? SetColor({0, 255, 255, 0}) : SetColor({255, 255, 0, 0}) : SetColor({ 255, 0, 255, 0 });
    return;
}

void Player::Draw(uint32_t buff[]) {
    // Calculate 4 dots for creating triangle-like player
    POINT d1 = { pos.x + size * cosf(dir), pos.y + size * sinf(dir) };
    POINT d2 = { pos.x + size * cosf(dir + 5 * M_PI / 6), pos.y + size * sinf(dir + 5 * M_PI / 6) };
    POINT d3 = { pos.x + 0.6 * size * cosf(dir + M_PI), pos.y + 0.6 * size * sinf(dir + M_PI) };
    POINT d4 = { pos.x + size * cosf(dir - 5 * M_PI / 6), pos.y + size * sinf(dir - 5 * M_PI / 6) };
    // Call Bresenham's line algorithm 4 times
    uint32_t color = GetColor();
    Bresenham(buff, d1, d2, color);
    Bresenham(buff, d2, d3, color);
    Bresenham(buff, d3, d4, color);
    Bresenham(buff, d4, d1, color);
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

POINT Player::GetSpeed() {
    return speed;
}

Bullet::Bullet(Player player) : GameObject() {
    SetSpeed(BULLETSPEED);
    SetDirection(atan2f(sinf(player.GetDirection()) * GetSpeed() + player.GetSpeed().y, cosf(player.GetDirection()) * GetSpeed() + player.GetSpeed().x));
    SetInitPosition(player);
    SetSize(BULLETSIZE);
    SetColor({ 255, 255, 255, 0 }); 
    ttl = BULLETTIME;
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

void Bullet::SetInitPosition(Player player) {
    SetPosition({ player.GetPosition().x + (player.GetSize() + GetSize()) * cosf(player.GetDirection()),  
                    player.GetPosition().y + (player.GetSize() + GetSize()) * sinf(player.GetDirection()) });
    return;
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

//void Asteroid::RecalculateDirection(float dir, POINT initSpeed, POINT futureSpeed) {
//    SetDirection(atan2f(initSpeed.y * cosf(dir) + futureSpeed.x * sinf(dir), futureSpeed.x * cosf(dir) + initSpeed.y * sinf(dir)));
//}


GameManager::GameManager() {
    levelDifficulties = { {5, 1, 0}, {3, 2, 1}, {1, 3, 2}, {1, 1, 4} };
    lifes = lifes2 = LIVES;
    level = 0;
    invincibleTime = invincibleTime2 = INVINCIBLETIME;
    time = time2 = 0;
    points = points2 = maxPoints = maxPoints2 = 0;
    player2 = Player(GameType::SIGLEPLAYER);
    player = Player(GameType::SIGLEPLAYER);
    state = GameState::GAME;
    return;
}

void GameManager::StartGame(GameType argType) {
    lifes = lifes2 = LIVES;
    points = points2 = 0;
    level = 0;
    totaltime = 0;
    time = time2 = 0;
    type = argType;
    state = GameState::GAME;
    player = Player(argType, true);
    player2 = Player(argType, false);
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
        GameWin();
        return;
    }
    time = 0;
    bullets.clear();
    bullets2.clear();
    player = Player(GetType(), true);
    player2 = Player(GetType(), false);
    StartLevel();
    return;
}

void GameManager::SetState(GameState argState) {
    state = argState;
}

GameState GameManager::GetState() const {
    return state;
}

GameType GameManager::GetType() const {
    return type;
}

void GameManager::SetBG(bool success) {
    hasBG = success;
}

bool GameManager::HasBG() {
    return hasBG;
}

void GameManager::GameOver() {
    asteroids.clear();
    bullets.clear();
    bullets2.clear();
    player.SetColor({ 0, 0, 0, 0 });
    player2.SetColor({ 0, 0, 0, 0 });
    if (GetType() == GameType::MULTIPLAYER) {
        points += points2;
    }
    if (points > maxPoints) {
        maxPoints = points;
    }
    SetState(GameState::GAMEOVER);
}

void GameManager::GameWin() {
    asteroids.clear();
    bullets.clear();
    bullets2.clear();
    player.SetColor({ 0, 0, 0, 0 });
    player2.SetColor({ 0, 0, 0, 0 });
    if (GetType() == GameType::SIGLEPLAYER) {
        points += GetLifes(true) * 10000;
    }
    else {
        points += (points2 + GetLifes(true) * 5000 + GetLifes(false) * 5000);
    }
    uint32_t tme = static_cast<uint32_t>(totaltime);
    if (tme <= 1000) {
        points += (1000 - tme) * 100;
    }
    if (points > maxPoints) {
        maxPoints = points;
    }
    SetState(GameState::GAMEWIN);
}

void GameManager::UpdateTimeGame(float dt) {
    totaltime += dt;
    DecreaseTime(time, dt);
    DecreaseTime(invincibleTime, dt);
    player.Move(dt);
    if (GetType() == GameType::MULTIPLAYER) {
        DecreaseTime(time2, dt);
        DecreaseTime(invincibleTime2, dt);
        player2.Move(dt);
    }
    for (auto it = bullets.begin(); it != bullets.end();) {
        it = (it->UpdateTime(dt)) ? bullets.erase(it) : ++it;
    }
    for (auto it = bullets2.begin(); it != bullets2.end();) {
        it = (it->UpdateTime(dt)) ? bullets2.erase(it) : ++it;
    }
    
    for (auto& x : asteroids) {
        x.Move(dt);
    }
    for (auto& x : bullets) {
        x.Move(dt);
    }
    for (auto& x : bullets2) {
        x.Move(dt);
    }
    // Collision between Player and Asteroids
    for (const auto& x : asteroids) {
        if (Distance(x.GetPosition(), player.GetPosition()) <= x.GetSize() + player.GetSize() * 0.6) {
            player = Player(GetType());
            if (invincibleTime == 0 && GetLifes(true)) {
                LostLife(true);
            }
            invincibleTime = INVINCIBLETIME;
        }
        if (GetType() == GameType::MULTIPLAYER) {
            if (Distance(x.GetPosition(), player2.GetPosition()) <= x.GetSize() + player2.GetSize() * 0.6) {
                player2 = Player(GetType(), false);
                if (invincibleTime2 == 0 && GetLifes(false)) {
                    LostLife(false);
                }
                invincibleTime2 = INVINCIBLETIME;
            }
        }
    }
    // Collision between Player and bullets
    //for (auto itB = bullets.begin(); itB != bullets.end(); itB++) {
    //    if (Distance(itB->GetPosition(), player.GetPosition()) <= itB->GetSize() + player.GetSize() * 0.6) {
    //        bullets.erase(itB);
    //        player = Player(GetType());
    //        lifes--;
    //        invincibleTime = INVINCIBLETIME;
    //        break;
    //    }
    //    if (GetType() == GameType::MULTIPLAYER) {
    //        if (Distance(itB->GetPosition(), player2.GetPosition()) <= itB->GetSize() + player2.GetSize() * 0.6) {
    //            bullets.erase(itB);
    //            player2 = Player(GetType(), false);
    //            lifes2--;
    //            invincibleTime2 = INVINCIBLETIME;
    //            break;
    //        }
    //    }
    //}
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
    if (GetType() == GameType::MULTIPLAYER) {
        for (auto itB = bullets2.begin(); itB != bullets2.end();) {
            bool next = true;
            for (auto itA = asteroids.begin(); itA != asteroids.end(); itA++) {
                if (Distance(itA->GetPosition(), itB->GetPosition()) <= itA->GetSize() + itB->GetSize()) {
                    auto t = Distance(itA->GetPosition(), itB->GetPosition());
                    next = false;
                    Asteroid parent = *itA;
                    itA = asteroids.erase(itA);
                    itB = bullets2.erase(itB);
                    if (parent.GetSizeType() != AsteroidSize::SMALL) {
                        asteroids.push_back(Asteroid(parent, false));
                        asteroids.push_back(Asteroid(parent, true));
                    }
                    points2 += (3 - static_cast<int64_t>(parent.GetSizeType())) * (pow(10, static_cast<int>(parent.GetSpeedType()))) * (level + 1);
                    break;
                }
            }
            if (next) {
                itB++;
            }
        }
    }
    //// Collision between asteroids
    //for (auto itB = asteroids.begin(); itB != asteroids.end(); itB++) {
    //    for (auto itA = next(itB); itA != asteroids.end(); itA++) {
    //        float delta = 5e-1;
    //        if (Distance(itA->GetPosition(), itB->GetPosition()) - itA->GetSize() - itB->GetSize() <= delta) {
    //            if (itA->GetSizeType() == itB->GetSizeType()) {
    //                float tmp = Distance(itA->GetPosition(), itB->GetPosition()) <= itA->GetSize() + itB->GetSize();
    //                float dir = CalculateDirection(itA->GetPosition(), itB->GetPosition());
    //                POINT speedTA = CalculateSpeed(itA->GetDirection(), itA->GetSpeed(), dir);
    //                POINT speedTB = CalculateSpeed(itB->GetDirection(), itB->GetSpeed(), dir);
    //                itA->RecalculateDirection(dir, speedTA, speedTB);
    //                itB->RecalculateDirection(dir, speedTB, speedTA);
    //            }
    //            else if (false) {
    //                
    //            }
    //        }
    //    }
    //}
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

int GameManager::GetLifes(bool first=true) const {
    return (first) ? lifes : lifes2;
}

void GameManager::LostLife(bool first) {
    (first) ? lifes-- : lifes2--;
    return;
}

bool GameManager::IsGameOver() const {
    return (GetType() == GameType::SIGLEPLAYER) ? !IsAlive(true) : !(IsAlive(true) || IsAlive(false));
}

bool GameManager::CanShoot(bool first = true) const {
    return (first) ? time == 0 : time2 == 0;
}

void GameManager::Shoot(bool first = true) {
    if (first) {
        time = PAUSETIME;
        bullets.push_back(Bullet(player));
    }
    else {
        time2 = PAUSETIME;
        bullets2.push_back(Bullet(player2));
    }
    return;
}

uint64_t GameManager::GetPoints(bool first=true) const {
    return (first) ? points: points2;
}

uint64_t GameManager::GetMaxPoints() const {
    return maxPoints;
}

bool GameManager::IsAlive(bool first = true) const {
    return (first) ? lifes : lifes2;
}

//
//  TODO:
//  Fix default BG
//  Move 2 players onto vector<Player>
//
//  IDEAS:
//  Collision detection (if asteroids - change direction [probably implement it in future])
//  Create death's animation
//

static GameManager gameManager;

// initialize game data in this function
void initialize() {
    srand(static_cast<unsigned int>(time(0)));
    gameManager = {};
    gameManager.SetState(GameState::MAINMENU);
    gameManager.SetBG(LoadDefaultBG(reinterpret_cast<uint32_t*>(defaultBG), "DefaultBG.txt"));
    return;
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt) {
    if (is_window_active()) {
        if (gameManager.GetState() == GameState::GAME) {
            gameManager.UpdateTimeGame(dt);
            if (is_key_pressed(VK_ESCAPE)) {
                gameManager.SetState(GameState::PAUSE);
            }
            if (gameManager.IsAlive(true)) {
                if (is_key_pressed(VK_LEFT) || gameManager.GetType() == GameType::SIGLEPLAYER && is_key_pressed('A')) {
                    gameManager.player.Rotate(-dt * ROTATIONSPEED);
                }
                if (is_key_pressed(VK_RIGHT) || gameManager.GetType() == GameType::SIGLEPLAYER && is_key_pressed('D')) {
                    gameManager.player.Rotate(dt * ROTATIONSPEED);
                }
                if (is_key_pressed(VK_UP) || gameManager.GetType() == GameType::SIGLEPLAYER && is_key_pressed('W')) {
                    gameManager.player.Accelerate(dt);
                }
                if (is_key_pressed(VK_SPACE) || gameManager.GetType() == GameType::SIGLEPLAYER && is_key_pressed('G')) {
                    if (gameManager.CanShoot()) {
                        gameManager.Shoot();
                    }
                }
            }
            if (gameManager.GetType() == GameType::MULTIPLAYER && gameManager.IsAlive(false)) {
                if (is_key_pressed('A')) {
                    gameManager.player2.Rotate(-dt * ROTATIONSPEED);
                }
                if (is_key_pressed('D')) {
                    gameManager.player2.Rotate(dt * ROTATIONSPEED);
                }
                if (is_key_pressed('W')) {
                    gameManager.player2.Accelerate(dt);
                }
                if (is_key_pressed('G')) {
                    if (gameManager.CanShoot(false)) {
                        gameManager.Shoot(false);
                    }
                }
            }
        }
        else if (gameManager.GetState() == GameState::PAUSE) {
            if (is_key_pressed('Q')) {
                gameManager.GameOver();
                gameManager.SetState(GameState::MAINMENU);
            }
            if (is_key_pressed('C')) {
                gameManager.SetState(GameState::GAME);
            }
        }
        else if (gameManager.GetState() == GameState::GAMEOVER || gameManager.GetState() == GameState::GAMEWIN) {
            if (is_key_pressed('Q')) {
                gameManager.GameOver();
                gameManager.SetState(GameState::MAINMENU);
            }
            if (is_key_pressed('F')) {
                gameManager.StartGame(gameManager.GetType());
            }
        }
        else if (gameManager.GetState() == GameState::MAINMENU) {
            if (is_key_pressed(VK_ESCAPE)) {
                schedule_quit_game();
            }
            if (is_key_pressed('S')) {
                gameManager.StartGame(GameType::SIGLEPLAYER);
            }
            if (is_key_pressed('M')) {
                gameManager.StartGame(GameType::MULTIPLAYER);
            }
        }
    }
    return;
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw() {
    // clear backbuffer
    if (gameManager.HasBG() && !(gameManager.GetState() == GameState::GAME || gameManager.GetState() == GameState::PAUSE)) {
        memcpy_s(buffer, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t), defaultBG, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
    }
    else {
        memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
    }
    if (gameManager.GetState() == GameState::GAME || gameManager.GetState() == GameState::PAUSE) {
        for (auto& x : gameManager.bullets) {
            x.Draw(reinterpret_cast<uint32_t*>(buffer));
        }
        for (auto& x : gameManager.bullets2) {
            x.Draw(reinterpret_cast<uint32_t*>(buffer));
        }
        for (auto& x : gameManager.asteroids) {
            x.Draw(reinterpret_cast<uint32_t*>(buffer));
        }
        if (gameManager.IsAlive(true)) {
            gameManager.player.Draw(reinterpret_cast<uint32_t*>(buffer));
        }
        if (gameManager.GetType() == GameType::MULTIPLAYER && gameManager.IsAlive(false)) {
            gameManager.player2.Draw(reinterpret_cast<uint32_t*>(buffer));
        }
        if (gameManager.GetState() == GameState::PAUSE) {
            DrawString(reinterpret_cast<uint32_t*>(buffer), "PAUSE", 200, SCREEN_HEIGHT / 2 - 50, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Press C to continue! ", 200, SCREEN_HEIGHT / 2 + 200);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Press Q to return to main menu! ", 200, SCREEN_HEIGHT / 2 + 150);
        }
        else if (gameManager.GetType() == GameType::SIGLEPLAYER) {
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Score: " + std::to_string(gameManager.GetPoints(true)), 10, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Highscore: " + std::to_string(gameManager.GetMaxPoints()), 400, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Lives: " + std::to_string(gameManager.GetLifes()), SCREEN_WIDTH - 140, 10);
        }
        else {
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Score: " + std::to_string(gameManager.GetPoints(false)), 10, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Score: " + std::to_string(gameManager.GetPoints(true)), 800, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Highscore: " + std::to_string(gameManager.GetMaxPoints()), 400, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Lives: " + std::to_string(gameManager.GetLifes(false)), 10, 60);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Lives: " + std::to_string(gameManager.GetLifes(true)), 800, 60);
        }
        return;
    }
    else if (gameManager.GetState() == GameState::GAMEOVER) {
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Game over!", 200, SCREEN_HEIGHT/2 - 50, 10);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Your score: " + std::to_string(gameManager.GetPoints(true)), 200, SCREEN_HEIGHT / 2 + 50);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Your highscore: " + std::to_string(gameManager.GetMaxPoints()), 200, SCREEN_HEIGHT / 2 + 100);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Press F to pay replay! ", 200, SCREEN_HEIGHT / 2 + 150);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Or press Q to give up ", 200, SCREEN_HEIGHT / 2 + 200);
    }
    else if (gameManager.GetState() == GameState::GAMEWIN) {
        DrawString(reinterpret_cast<uint32_t*>(buffer), "UNBELIEVABLE!", 200, SCREEN_HEIGHT / 2 - 50, 10);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Your score: " + std::to_string(gameManager.GetPoints(true)), 200, SCREEN_HEIGHT / 2 + 50);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Your highscore: " + std::to_string(gameManager.GetMaxPoints()), 200, SCREEN_HEIGHT / 2 + 100);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Press F to pay replay! ", 200, SCREEN_HEIGHT / 2 + 150);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Or press q to leave as a winner ", 200, SCREEN_HEIGHT / 2 + 200);
    }
    else if (gameManager.GetState() == GameState::MAINMENU) {
        DrawString(reinterpret_cast<uint32_t*>(buffer), "COSMOSHOOTING", 175, 150, 10);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "[S]ingleplayer or [M]ultiplayer", 200, SCREEN_HEIGHT / 2 - 100, 5);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Press UP and W to accelerate", 300, SCREEN_HEIGHT / 2 + 100, 3);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Press LEFTRIGHT and AD to rotate", 300, SCREEN_HEIGHT / 2 + 150, 3);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Press SPACE and G to shoot", 300, SCREEN_HEIGHT / 2 + 200, 3);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Created by lumidelta\a and based on Atari 1979 ", 300, 730, 2);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "6+", 10, 730, 4);
    }
}

// free game data in this function
void finalize() {
    return;
}