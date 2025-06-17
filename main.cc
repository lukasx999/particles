#include <cassert>
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
    Vector2 m_gravity { 0, 500 };
    static constexpr float m_radius = 50;

public:
    Particle(Vector2 pos)
    : m_pos(pos)
    { }

    void update(float dt) {
        m_pos = Vector2Add(m_pos, Vector2Scale(m_vel, dt));
        m_vel = Vector2Add(m_vel, Vector2Scale(m_acc, dt));
        // m_vel = Vector2Add(m_vel, Vector2Scale(m_gravity, dt));
    }

    void draw() const {
        DrawCircleV(m_pos, m_radius, BLUE);

        #ifdef DEBUG
        float line_size = 3;
        DrawLineEx(m_pos, Vector2Add(m_pos, m_vel), line_size, RED);
        DrawLineEx(m_pos, Vector2Add(m_pos, m_acc), line_size, GREEN);
        DrawLineEx(m_pos, Vector2Add(m_pos, m_gravity), line_size, PURPLE);
        DrawText(std::format("pos: {}, {}", trunc(m_pos.x), trunc(m_pos.y)).c_str(), 0, 0, 30, WHITE);
        DrawText(std::format("vel: {}, {}", trunc(m_vel.x), trunc(m_vel.y)).c_str(), 0, 30, 30, WHITE);
        DrawText(std::format("acc: {}, {}", trunc(m_acc.x), trunc(m_acc.y)).c_str(), 0, 60, 30, WHITE);
        #endif // DEBUG
    }

    void resolve_collisions() {
        assert(m_pos.x < WIDTH);
        assert(m_pos.y < HEIGHT);
        assert(m_pos.y > 0);
        assert(m_pos.x > 0);

        float damp_factor = 0.95;

        if (m_pos.y-m_radius <= 0) {
            m_vel.y *= -1;
        }

        if (m_pos.y+m_radius >= HEIGHT) {
            m_vel.y *= -1;
        }

        if (m_pos.x+m_radius >= WIDTH) {
            m_vel.x *= -damp_factor;
            // m_acc.x *= 0.9;
        }

        if (m_pos.x-m_radius <= 0) {
            m_vel.x *= -1;
        }

    }

    void apply_force_absolute(Vector2 pos) {
        m_acc = Vector2Subtract(pos, m_pos);
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

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                p.apply_force_absolute(GetMousePosition());

            p.resolve_collisions();
            p.update(GetFrameTime());
            p.draw();
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
