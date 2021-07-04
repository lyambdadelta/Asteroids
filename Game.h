#pragma once
#include "Engine.h"
#include <vector>

enum class AsteroidSpeed {
    SLOW,
    MEDIUM,
    FAST
};

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

class GameObject {
public:
    GameObject();
    float GetSize() const;
    float GetSpeed() const;
    float GetDirection() const;
    POINT GetPosition() const;
    void Rotate(float angle);
    virtual void Draw(uint32_t buff[]) {}
    virtual void Move(float dt) {}
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
private:
};

class Bullet : public GameObject {
public:
    Bullet(Player player);
private:
    void SetInitPosition(Player player);
    float ttl;
};

class Asteroid : public GameObject {
public:
    Asteroid(AsteroidSpeed argSpeed, AsteroidSize argSize);
    void Draw(uint32_t buff[]) override;
    void Move(float dt) override;
private:
    AsteroidSpeed speedType;
    AsteroidSize sizeType;
    void SetInitSize(AsteroidSize argSize);
    void SetInitSpeed(AsteroidSpeed argSpeed);
    void SetInitDirection();
    void SetInitPosition();
    void SetInitColor(AsteroidSpeed argSpeed);
};

class GameManager {
public:
    Player player;
    std::vector<Asteroid> asteroids;
    std::vector<Bullet> bullets;

    std::vector<std::vector<int>> levelDifficulties;

    GameManager();

    void StartLevel();
    bool IsLevelOver();
    int GetLifes();
    void LostLife();
    bool IsGameOver();
private:
    uint64_t points;
    int lifes;
    int level;
};