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

struct Point {
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

void Bresenham(uint32_t buff[], Point d1, Point d2, uint32_t color);
float Distance(Point a, Point b);
void DrawString(uint32_t buff[], std::string str, uint32_t posx, uint32_t posy, uint32_t size);
int mod(int value, int m);

//float CalculateDirection(Point a, Point b);
//Point CalculateSpeed(float initDir, float speed, float collisionDir);


class GameObject {
public:
    GameObject();

    // Info
    uint32_t GetColor() const;
    float GetDirection() const;
    Point GetPosition() const;
    float GetSize() const;
    float GetSpeed() const;
    
    // Action
    void Rotate(float angle);
    virtual void Move(float dt);

    virtual void Draw(uint32_t buff[]) const;
    
protected:
    float dir, size, speed;
    Point pos;
    BGRA color;

    // Set
    void SetColor(BGRA argColor);
    void SetSpeed(float argSpeed);
    void SetDirection(float argDir);
    void SetSize(float argSize);
    void SetPosition(Point argPosition);
};


class Player : public GameObject {
public:
    class Bullet : public GameObject {
    public:
        Bullet(Player player);
        bool UpdateTime(float dt);
    private:
        void SetInitPosition(Player player);
        float ttl;
    };

    std::list<Bullet> bullets;

    Player(GameType argType, bool first);
    
    // Info
    bool CanShoot() const;
    uint32_t GetLifes() const;
    uint64_t GetPoints() const;
    Point GetSpeed() const;
    bool IsAlive() const;

    // Action
    void Accelerate(float dt);
    void AddPoints(uint64_t points);
    void Move(float dt) override;
    void Shoot();
    void UpdateTime(float dt);

    // Reset    
    void Reset();
    void Collision();

    void Draw(uint32_t buff[]) const override;
private:
    // Due to acceleration it is easier to store sped as x and y values,
    // not as speed and direction
    float invincibleTime, time;
    uint32_t lifes;
    uint64_t points;
    Point initPos, speed;

    void DecreaseTime(float& t, float dt);
    void SetSpeed(Point argSpeed);
};

class Asteroid : public GameObject {
public:
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

    Asteroid(AsteroidSpeed argSpeed, AsteroidSize argSize);
    Asteroid(const Asteroid& prev, bool type);

    // Info
    AsteroidSpeed GetSpeedType() const;
    AsteroidSize GetSizeType() const;
    //void RecalculateDirection(float dir, Point initSpeed, Point futureSpeed);
private:
    AsteroidSize sizeType;
    AsteroidSpeed speedType;

    // Set
    void SetInitColor(AsteroidSpeed argSpeed);
    void SetInitDirection();
    void SetInitPosition();
    void SetInitSize(AsteroidSize argSize);
    void SetInitSpeed(AsteroidSpeed argSpeed);
};

// Manager for the game that controls situation on the field
class GameManager {
public:
    std::vector<Player> players;
    std::vector<Asteroid> asteroids;

    GameManager();

    // Info
    uint64_t GetMaxPoints() const;
    uint64_t GetPoints() const;
    GameState GetState() const;
    GameType GetType() const;
    bool HasBG() const;
    bool IsGameOver() const;
    bool IsLevelOver() const;

    // Update game states
    void GameOver();
    void GameWin();
    void NextLevel();
    void SetState(GameState argState);
    void StartGame(GameType argType);
    void StartLevel();
    void UpdateTimeGame(float dt);
    
    void LoadDefaultBG(uint32_t buff[], std::string name);
private:
    std::vector<std::vector<int>> levelDifficulties;
    GameState state;
    GameType type;
    uint64_t maxPoints, points;
    bool hasBG;
    uint32_t level;
    float totaltime;
};