#include <print>
#include <format>

#include <raylib.h>
#include <raymath.h>

#define DEBUG

static constexpr auto WIDTH = 1600;
static constexpr auto HEIGHT = 900;

enum class Direction { Left, Right, Up, Down };

class Particle {
    Vector2 m_pos;
    Vector2 m_vel { 0, 0 };
    Vector2 m_acc { 0, 0 };
    Vector2 m_gravity { 0, 300 };
    static constexpr float m_radius = 50;

public:
    Particle(Vector2 pos)
    : m_pos(pos)
    { }

    void update(float dt) {
        m_pos = Vector2Add(m_pos, Vector2Scale(m_vel, dt));
        m_vel = Vector2Add(m_vel, Vector2Scale(m_acc, dt));
        m_vel = Vector2Add(m_vel, Vector2Scale(m_gravity, dt));
    }

    void draw() const {
        DrawCircleV(m_pos, m_radius, BLUE);

        #ifdef DEBUG
        DrawText(std::format("pos: {}, {}", m_pos.x, m_pos.y).c_str(), 0, 0, 30, WHITE);
        DrawText(std::format("vel: {}, {}", m_vel.x, m_vel.y).c_str(), 0, 30, 30, WHITE);
        DrawText(std::format("acc: {}, {}", m_acc.x, m_acc.y).c_str(), 0, 60, 30, WHITE);
        #endif // DEBUG
    }

    void apply_force(Direction dir) {
        float force = 100;
        switch (dir) {
            case Direction::Left:
                m_acc.x -= force;
                break;
            case Direction::Right:
                m_acc.x += force;
                break;
            case Direction::Up:
                m_acc.y -= force;
                break;
            case Direction::Down:
                m_acc.y += force;
                break;
        }
    }

    void resolve_collisions() {

        if (m_pos.y-m_radius/2.0f <= 0) {
            m_vel.y *= -1;
        }

        if (m_pos.y+m_radius/2.0f >= HEIGHT) {
            m_vel.y *= -1;
        }

        if (m_pos.x+m_radius/2.0f >= WIDTH) {
            m_vel.x *= -1;
        }

        if (m_pos.x-m_radius/2.0f <= 0) {
            m_vel.x *= -1;
        }

    }

};

int main() {

    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "particles");

    Particle p({ WIDTH/2.0f, HEIGHT/2.0f });

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(BLACK);

            if (IsKeyPressed(KEY_RIGHT))
                p.apply_force(Direction::Right);

            if (IsKeyPressed(KEY_LEFT))
                p.apply_force(Direction::Left);

            if (IsKeyPressed(KEY_UP))
                p.apply_force(Direction::Up);

            if (IsKeyPressed(KEY_DOWN))
                p.apply_force(Direction::Down);

            p.resolve_collisions();
            p.update(GetFrameTime());
            p.draw();
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
