#pragma once
#include "Engine.h"
#include <string>
#include <vector>
#include <list>

enum class GameType {
    SIGLEPLAYER,
    MULTIPLAYER
};

// Diferent states of the game
enum class GameState {
    MAINMENU,
    GAME,
    PAUSE,
    GAMEOVER,
    GAMEWIN
};

// Different colors for different Speed-type asteroids
enum class AsteroidSpeed {
    SLOW,
    MEDIUM,
    FAST
};

// Different stage of Asteroid's life
enum class AsteroidSize {
    SMALL,
    NORMAL,
    BIG
};

struct POINT {
    float x;
    float y;
};

struct BGRA {
    uint8_t alpha;
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    BGRA();
    BGRA(uint8_t b, uint8_t g, uint8_t r, uint8_t a);
    uint32_t GetInt() const;
};

float Distance(POINT a, POINT b);
int mod(int value, int m);
void Bresenham(uint32_t buff[], POINT d1, POINT d2, uint32_t color);
void DecreaseTime(float& t, float dt);
void DrawString(uint32_t buff[], std::string str, uint32_t posx, uint32_t posy, uint32_t size);
bool LoadDefaultBG(uint32_t buff[], std::string name);
//float CalculateDirection(POINT a, POINT b);
//POINT CalculateSpeed(float initDir, float speed, float collisionDir);

class GameObject {
public:
    GameObject();
    float GetSize() const;
    float GetSpeed() const;
    float GetDirection() const;
    POINT GetPosition() const;
    uint32_t GetColor() const;
    void Rotate(float angle);
    virtual void Draw(uint32_t buff[]);
    virtual void Move(float dt);
    void SetColor(BGRA argColor);
protected:
    void SetDirection(float argDir);
    void SetSize(float argSize);
    void SetSpeed(float argSpeed);
    void SetPosition(POINT argPosition);
    float size;
    float speed;
    float dir;
    POINT pos;
    BGRA color;
};

class Player : public GameObject {
public:
    Player();
    Player(GameType argType, bool first);
    void Draw(uint32_t buff[]) override;
    void Move(float dt) override;
    void Accelerate(float dt);
    POINT GetSpeed();
private:
    // Due to acceleration it is easier to store sped as x and y values,
    // not as speed and direction
    POINT speed;
};

class Bullet : public GameObject {
public:
    Bullet(Player player);
    bool UpdateTime(float dt);
private:
    void SetInitPosition(Player player);
    float ttl;
};

class Asteroid : public GameObject {
public:
    Asteroid(AsteroidSpeed argSpeed, AsteroidSize argSize);
    Asteroid(const Asteroid& prev, bool type);
    AsteroidSpeed GetSpeedType() const;
    AsteroidSize GetSizeType() const;
    //void RecalculateDirection(float dir, POINT initSpeed, POINT futureSpeed);
private:
    AsteroidSpeed speedType;
    AsteroidSize sizeType;
    void SetInitSize(AsteroidSize argSize);
    void SetInitSpeed(AsteroidSpeed argSpeed);
    void SetInitDirection();
    void SetInitPosition();
    void SetInitColor(AsteroidSpeed argSpeed);
};

// Manager for the game that controls situation on the field
class GameManager {
public:
    Player player;
    Player player2;
    std::vector<Asteroid> asteroids;
    std::list<Bullet> bullets;
    std::list<Bullet> bullets2;

    std::vector<std::vector<int>> levelDifficulties;

    GameManager();

    void UpdateTimeGame(float dt);
    void StartGame(GameType argType);
    void StartLevel();
    void NextLevel();
    void GameOver();
    void GameWin();
    bool IsLevelOver() const;
    int GetLifes(bool first) const;
    void LostLife(bool first);
    GameState GetState() const;
    GameType GetType() const;
    bool IsGameOver() const;
    bool CanShoot(bool first) const;
    uint64_t GetPoints(bool first) const;
    uint64_t GetMaxPoints() const;
    void Shoot(bool first);
    bool IsAlive(bool first) const;
    void SetState(GameState argState);
    void SetBG(bool success);
    bool HasBG();
private:
    GameState state;
    GameType type;
    uint64_t points, points2;
    uint64_t maxPoints, maxPoints2;
    bool hasBG;
    int lifes, lifes2;
    int level;
    float time, time2;
    float totaltime;
    float invincibleTime, invincibleTime2;
};