#pragma once
#include "Engine.h"
#include <vector>
#include <list>

// Diferent states of the game
enum class GameState {
    MAINMENU,
    LEADERBOARD,
    GAME
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
protected:
    void SetColor(BGRA argColor);
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
    Asteroid(Asteroid prev, bool type);
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

    void UpdateTime(float dt);
    void StartLevel();
    bool IsLevelOver() const;
    int GetLifes() const;
    void LostLife();
    bool IsGameOver() const;
    bool CanShoot() const;
    void Shoot();
private:
    GameState state;
    uint64_t points;
    int lifes;
    int level;
    float time;
};