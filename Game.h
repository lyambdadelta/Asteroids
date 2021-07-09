#pragma once
#include "Engine.h"
#include <string>
#include <vector>
#include <list>

// Diferent states of the game
enum class GameState {
    MAINMENU,
    LEADERBOARD,
    GAME,
    PAUSE,
    GAMEOVER
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
void Bresenham(uint32_t buff[], POINT d1, POINT d2);
void DecreaseTime(float& t, float dt);
void DrawString(uint32_t buff[], std::string str, uint32_t posx, uint32_t posy, uint32_t size);

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
    std::vector<Asteroid> asteroids;
    std::list<Bullet> bullets;

    std::vector<std::vector<int>> levelDifficulties;

    GameManager();

    void UpdateTimeGame(float dt);
    void RestartGame();
    void StartLevel();
    void NextLevel();
    void GameOver();
    bool IsLevelOver() const;
    int GetLifes() const;
    void LostLife();
    GameState GetState() const;
    bool IsGameOver() const;
    bool CanShoot() const;
    uint64_t GetPoints() const;
    uint64_t GetMaxPoints() const;
    void Shoot();
    void setState(GameState argState);
private:
    GameState state;
    uint64_t points;
    uint64_t maxPoints;
    int lifes;
    int level;
    float time;
    float invincibleTime;
};