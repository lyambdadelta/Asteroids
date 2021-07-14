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

constexpr float M_PI = 3.141592f;
constexpr float ROTATIONSPEED = 10.0f;
constexpr float ACCELERATION = 50.0f;
constexpr float MAXSPEED = 120.0f;
constexpr float BULLETSPEED = 200.0f;
constexpr float BULLETTIME = 3.0f;
constexpr float BULLETSIZE = 3.0f;
constexpr float NONCREATIONRADIUS = 300.0f;
constexpr int LIVES = 3;
constexpr float SIZE = 15.0f;
constexpr float PAUSETIME = 0.01f;
constexpr float INVINCIBLETIME = 3.0f;
static Point INIT_POS = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
static Point INIT_POS1 = { SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2 };
static Point INIT_POS2 = { 2 * SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2 };

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

float Distance(Point a, Point b) {
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


int mod(int value, int m) {
    value %= m;
    return (value >= 0) ? value : value + m;
}

void Bresenham(uint32_t buff[], Point d1, Point d2, uint32_t color) {
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

//float CalculateDirection(Point a, Point b) {
//    return atan2f(a.x - b.x, a.y - b.y);
//}

//Point CalculateSpeed(float initDir, float speed, float collisionDir) {
//    return { speed * cosf(collisionDir - initDir), speed * sinf(collisionDir - initDir) };
//}

// Class GameObject
// Public GameObject
GameObject::GameObject() {
    SetDirection(0.0);
    SetSize(0.0);
    SetSpeed(0.0);
    SetColor({ 0, 0, 0, 0 });
}

// Public GameObject info
uint32_t GameObject::GetColor() const {
    return color.GetInt();
}

float GameObject::GetDirection() const {
    return dir;
}

Point GameObject::GetPosition() const {
    return pos;
}

float GameObject::GetSize() const {
    return size;
}

float GameObject::GetSpeed() const {
    return speed;
}

// Public GameObject action
void GameObject::Rotate(float angle) {
    dir += angle;
    dir = fmod(dir, 2 * M_PI);
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

void GameObject::Draw(uint32_t buff[]) const {
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

// Protected GameObject set
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

void GameObject::SetPosition(Point argPosition) {
    pos = argPosition;
    return;
}

// Class Bullet in class Player
Player::Bullet::Bullet(Player player) : GameObject() {
    SetSpeed(BULLETSPEED);
    SetDirection(atan2f(sinf(player.GetDirection()) * GetSpeed() + player.GetSpeed().y, cosf(player.GetDirection()) * GetSpeed() + player.GetSpeed().x));
    SetInitPosition(player);
    SetSize(BULLETSIZE);
    SetColor({ 255, 255, 255, 0 });
    ttl = BULLETTIME;
    return;
}

// Public Bullet Update
bool Player::Bullet::UpdateTime(float dt) {
    ttl -= dt;
    if (ttl <= 0) {
        ttl = 0;
        return true;
    }
    return false;
}

void Player::Bullet::SetInitPosition(Player player) {
    SetPosition({ player.GetPosition().x + (player.GetSize() + GetSize()) * cosf(player.GetDirection()),
                    player.GetPosition().y + (player.GetSize() + GetSize()) * sinf(player.GetDirection()) });
    return;
}

// Class Player
Player::Player() {}

Player::Player(GameType argType, bool first=true) {
    SetPosition((argType == GameType::SIGLEPLAYER) ? INIT_POS : (first) ? INIT_POS2 : INIT_POS1);
    initPos = GetPosition();
    SetDirection(- M_PI / 2);
    SetSize(SIZE);
    SetSpeed({0, 0});
    lifes = LIVES;
    invincibleTime = INVINCIBLETIME;
    points = 0;
    (argType == GameType::MULTIPLAYER) ? (first) ? SetColor({0, 255, 255, 0}) : SetColor({255, 255, 0, 0}) : SetColor({ 255, 0, 255, 0 });
    time = 0;
    return;
}

// Public Player info 
bool Player::CanShoot() const {
    return time == 0;
}

int Player::GetLifes() const {
    return lifes;
}

uint64_t Player::GetPoints() const {
    return points;
}

Point Player::GetSpeed() const {
    return speed;
}

bool Player::IsAlive() const {
    return lifes;
}

// Public Player action 
void Player::Accelerate(float dt) {
    Point newSpeed = speed;
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

void Player::AddPoints(uint64_t argPoints) {
    points += argPoints;
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

void Player::Shoot() {
    time = PAUSETIME;
    bullets.push_back(Bullet(*this));
    return;
}

void Player::UpdateTime(float dt) {
    DecreaseTime(time, dt);
    DecreaseTime(invincibleTime, dt);
    return;
}

// Public Player reset 
void Player::Reset() {
    bullets.clear();
    SetPosition(initPos);
    SetDirection(-M_PI / 2);
    SetSpeed({ 0, 0 });
    time = 0;
    return;
}

void Player::Collision() {
    if (invincibleTime == 0 && lifes) {
        lifes--;
    }
    invincibleTime = INVINCIBLETIME;
    Reset();
    return;
}


void Player::Draw(uint32_t buff[]) const {
    // Calculate 4 dots for creating triangle-like player
    Point d1 = { pos.x + size * cosf(dir), pos.y + size * sinf(dir) };
    Point d2 = { pos.x + size * cosf(dir + 5 * M_PI / 6), pos.y + size * sinf(dir + 5 * M_PI / 6) };
    Point d3 = { pos.x + 0.6 * size * cosf(dir + M_PI), pos.y + 0.6 * size * sinf(dir + M_PI) };
    Point d4 = { pos.x + size * cosf(dir - 5 * M_PI / 6), pos.y + size * sinf(dir - 5 * M_PI / 6) };
    // Call Bresenham's line algorithm 4 times
    uint32_t color = GetColor();
    Bresenham(buff, d1, d2, color);
    Bresenham(buff, d2, d3, color);
    Bresenham(buff, d3, d4, color);
    Bresenham(buff, d4, d1, color);
    return;
}

// Private Player
void Player::SetSpeed(Point argSpeed) {
    speed = argSpeed;
}

void Player::DecreaseTime(float& t, float dt) {
    if (t != 0) {
        t = (t - dt <= 0) ? 0.0f : t - dt;
    }
}

// Class Asteroid
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

// For destroy purposes
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

// Public Asteroid info 
Asteroid::AsteroidSize Asteroid::GetSizeType() const {
    return sizeType;
}

Asteroid::AsteroidSpeed Asteroid::GetSpeedType() const {
    return speedType;
}

// Private Asteroid set 
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

void Asteroid::SetInitDirection() {
    SetDirection(static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX) * 2 * M_PI);
    return;
}

void Asteroid::SetInitPosition() {
    Point argPos;
    do {
        argPos = { static_cast <float> (std::rand() % SCREEN_WIDTH), static_cast <float> (std::rand() % SCREEN_HEIGHT) };
    } while (Distance(argPos, INIT_POS) < NONCREATIONRADIUS);
    SetPosition(argPos);
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

//void Asteroid::RecalculateDirection(float dir, Point initSpeed, Point futureSpeed) {
//    SetDirection(atan2f(initSpeed.y * cosf(dir) + futureSpeed.x * sinf(dir), futureSpeed.x * cosf(dir) + initSpeed.y * sinf(dir)));
//}

// Class GameManager
GameManager::GameManager() {
    levelDifficulties = { {5, 1, 0}, {3, 2, 1}, {1, 3, 2}, {1, 1, 4} };
    level = 0;
    maxPoints = 0;
    players = std::vector<Player>();
    state = GameState::GAME;
    return;
}

// Public GameManager info 
uint64_t GameManager::GetMaxPoints() const {
    return maxPoints;
}

uint64_t GameManager::GetPoints() const {
    return points;
}

GameState GameManager::GetState() const {
    return state;
}

GameType GameManager::GetType() const {
    return type;
}

bool GameManager::HasBG() const {
    return hasBG;
}

bool GameManager::IsLevelOver() const {
    return asteroids.empty();
}

bool GameManager::IsGameOver() const {
    bool anyAlive = false;
    for (const auto& x : players) {
        anyAlive |= x.IsAlive();
    }
    return !anyAlive;
}

// Public GameManager update 
void GameManager::GameOver() {
    asteroids.clear();
    points = 0;
    for (const auto& x : players) {
        points += x.GetPoints();
    }
    if (points > maxPoints) {
        maxPoints = points;
    }
    players.clear();
    SetState(GameState::GAMEOVER);
}

void GameManager::GameWin() {
    asteroids.clear();
    points = 0;
    for (const auto& x : players) {
        points += x.GetPoints();
        points += x.GetLifes() * 10000;
    }
    players.clear();
    uint32_t tme = static_cast<uint32_t>(totaltime);
    if (tme <= 1000) {
        points += (1000 - tme) * 100;
    }
    if (points > maxPoints) {
        maxPoints = points;
    }
    SetState(GameState::GAMEWIN);
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
    for (auto& x : players) {
        x.Reset();
    }
    StartLevel();
    return;
}

void GameManager::SetState(GameState argState) {
    state = argState;
}

void GameManager::StartGame(GameType argType) {
    level = 0;
    totaltime = 0;
    type = argType;
    state = GameState::GAME;
    players.clear();
    asteroids.clear();
    players.push_back(Player({ type }));
    if (type == GameType::MULTIPLAYER) {
        players.push_back(Player({ type, false }));
    }
    StartLevel();
    return;
}

void GameManager::StartLevel() {
    for (int i = 0; i < levelDifficulties[level].size(); i++) {
        for (int j = 0; j < levelDifficulties[level][i]; j++) {
            asteroids.push_back(Asteroid(static_cast<Asteroid::AsteroidSpeed>(i), Asteroid::AsteroidSize::BIG));
        }
    }
    return;
}

void GameManager::UpdateTimeGame(float dt) {
    totaltime += dt;
    for (auto& x : players) {
        x.UpdateTime(dt);
        x.Move(dt);
        for (auto it = x.bullets.begin(); it != x.bullets.end();) {
            it->Move(dt);
            it = (it->UpdateTime(dt)) ? x.bullets.erase(it) : ++it;
        }
    }

    for (auto& x : asteroids) {
        x.Move(dt);
    }
    // Collision between Player and Asteroids
    for (const auto& x : asteroids) {
        for (auto& player : players) {
            if (Distance(x.GetPosition(), player.GetPosition()) <= x.GetSize() + player.GetSize() * 0.6) {
                player.Collision();
            }
        }
    }
    // Collision between Player and bullets
    // Work, but not funny with it
    for (auto& player: players) {
        for (auto itB = player.bullets.begin(); itB != player.bullets.end(); itB++) {
            if (Distance(itB->GetPosition(), player.GetPosition()) <= itB->GetSize() + player.GetSize() * 0.6) {
                player.bullets.erase(itB);
                player.Collision();
                break;
            }
        }
    }
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
    for (auto& x : players) {
        for (auto itB = x.bullets.begin(); itB != x.bullets.end();) {
            bool next = true;
            for (auto itA = asteroids.begin(); itA != asteroids.end(); itA++) {
                if (Distance(itA->GetPosition(), itB->GetPosition()) <= itA->GetSize() + itB->GetSize()) {
                    auto t = Distance(itA->GetPosition(), itB->GetPosition());
                    next = false;
                    Asteroid parent = *itA;
                    itA = asteroids.erase(itA);
                    itB = x.bullets.erase(itB);
                    if (parent.GetSizeType() != Asteroid::AsteroidSize::SMALL) {
                        asteroids.push_back(Asteroid(parent, false));
                        asteroids.push_back(Asteroid(parent, true));
                    }
                    x.AddPoints((3 - static_cast<int64_t>(parent.GetSizeType())) * (pow(10, static_cast<int>(parent.GetSpeedType()))) * (level + 1));
                    break;
                }
            }
            if (next) {
                itB++;
            }
        }
    }
    // Collision between asteroids
    // Didn't debugged, not funny with it
    //for (auto itB = asteroids.begin(); itB != asteroids.end(); itB++) {
    //    for (auto itA = next(itB); itA != asteroids.end(); itA++) {
    //        float delta = 5e-1;
    //        if (Distance(itA->GetPosition(), itB->GetPosition()) - itA->GetSize() - itB->GetSize() <= delta) {
    //            if (itA->GetSizeType() == itB->GetSizeType()) {
    //                float tmp = Distance(itA->GetPosition(), itB->GetPosition()) <= itA->GetSize() + itB->GetSize();
    //                float dir = CalculateDirection(itA->GetPosition(), itB->GetPosition());
    //                Point speedTA = CalculateSpeed(itA->GetDirection(), itA->GetSpeed(), dir);
    //                Point speedTB = CalculateSpeed(itB->GetDirection(), itB->GetSpeed(), dir);
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


void GameManager::LoadDefaultBG(uint32_t buff[], std::string name) {
    std::ifstream input(name);
    unsigned counter = 0;
    if (input.is_open()) {
        for (int i = 9; i < SCREEN_HEIGHT - 9; i++) {
            for (int j = 12; j < SCREEN_WIDTH - 12; j++) {
                input >> buff[i * SCREEN_WIDTH + j];
                counter++;
            }
        }
        input.close();
    }
    // After fix BG change to SCREEN_HEIGHT * SCREEN_WIDTH
    hasBG = (counter == 750 * 1000) ? true : false;
    return;
}


//
//  TODO:
//  Fix default BG
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
    gameManager.LoadDefaultBG(reinterpret_cast<uint32_t*>(defaultBG), "DefaultBG.txt");
    return;
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt) {
    if (is_window_active()) {
        if (gameManager.GetState() == GameState::GAME) {
            if (is_key_pressed(VK_ESCAPE)) {
                gameManager.SetState(GameState::PAUSE);
            }
            if (gameManager.players[0].IsAlive()) {
                if (is_key_pressed(VK_LEFT) || gameManager.GetType() == GameType::SIGLEPLAYER && is_key_pressed('A')) {
                    gameManager.players[0].Rotate(-dt * ROTATIONSPEED);
                }
                if (is_key_pressed(VK_RIGHT) || gameManager.GetType() == GameType::SIGLEPLAYER && is_key_pressed('D')) {
                    gameManager.players[0].Rotate(dt * ROTATIONSPEED);
                }
                if (is_key_pressed(VK_UP) || gameManager.GetType() == GameType::SIGLEPLAYER && is_key_pressed('W')) {
                    gameManager.players[0].Accelerate(dt);
                }
                if (is_key_pressed(VK_SPACE) || gameManager.GetType() == GameType::SIGLEPLAYER && is_key_pressed('G')) {
                    if (gameManager.players[0].CanShoot()) {
                        gameManager.players[0].Shoot();
                    }
                }
            }
            if (gameManager.GetType() == GameType::MULTIPLAYER && gameManager.players[1].IsAlive()) {
                if (is_key_pressed('A')) {
                    gameManager.players[1].Rotate(-dt * ROTATIONSPEED);
                }
                if (is_key_pressed('D')) {
                    gameManager.players[1].Rotate(dt * ROTATIONSPEED);
                }
                if (is_key_pressed('W')) {
                    gameManager.players[1].Accelerate(dt);
                }
                if (is_key_pressed('G')) {
                    if (gameManager.players[1].CanShoot()) {
                        gameManager.players[1].Shoot();
                    }
                }
            }
            gameManager.UpdateTimeGame(dt);
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
        for (const auto& player : gameManager.players) {
            if (player.IsAlive()) {
                player.Draw(reinterpret_cast<uint32_t*>(buffer));
            }
            for (auto& x : player.bullets) {
                x.Draw(reinterpret_cast<uint32_t*>(buffer));
            }
        }
        for (auto& x : gameManager.asteroids) {
            x.Draw(reinterpret_cast<uint32_t*>(buffer));
        }
        if (gameManager.GetState() == GameState::PAUSE) {
            DrawString(reinterpret_cast<uint32_t*>(buffer), "PAUSE", 200, SCREEN_HEIGHT / 2 - 50, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Press C to continue! ", 200, SCREEN_HEIGHT / 2 + 200);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Press Q to return to main menu! ", 200, SCREEN_HEIGHT / 2 + 150);
        }
        else if (gameManager.GetType() == GameType::SIGLEPLAYER) {
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Score: " + std::to_string(gameManager.players[0].GetPoints()), 10, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Highscore: " + std::to_string(gameManager.GetMaxPoints()), 400, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Lives: " + std::to_string(gameManager.players[0].GetLifes()), SCREEN_WIDTH - 140, 10);
        }
        else {
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Score: " + std::to_string(gameManager.players[1].GetPoints()), 10, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Score: " + std::to_string(gameManager.players[0].GetPoints()), 800, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Highscore: " + std::to_string(gameManager.GetMaxPoints()), 400, 10);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Lives: " + std::to_string(gameManager.players[1].GetLifes()), 10, 60);
            DrawString(reinterpret_cast<uint32_t*>(buffer), "Lives: " + std::to_string(gameManager.players[0].GetLifes()), 800, 60);
        }
        return;
    }
    else if (gameManager.GetState() == GameState::GAMEOVER) {
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Game over!", 200, SCREEN_HEIGHT/2 - 50, 10);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Your score: " + std::to_string(gameManager.GetPoints()), 200, SCREEN_HEIGHT / 2 + 50);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Your highscore: " + std::to_string(gameManager.GetMaxPoints()), 200, SCREEN_HEIGHT / 2 + 100);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Press F to pay replay! ", 200, SCREEN_HEIGHT / 2 + 150);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Or press Q to give up ", 200, SCREEN_HEIGHT / 2 + 200);
    }
    else if (gameManager.GetState() == GameState::GAMEWIN) {
        DrawString(reinterpret_cast<uint32_t*>(buffer), "UNBELIEVABLE!", 200, SCREEN_HEIGHT / 2 - 50, 10);
        DrawString(reinterpret_cast<uint32_t*>(buffer), "Your score: " + std::to_string(gameManager.GetPoints()), 200, SCREEN_HEIGHT / 2 + 50);
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