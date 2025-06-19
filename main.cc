#include <cassert>
#include <print>
#include <format>
#include <random>
#include <span>

#include <raylib.h>
#include <raymath.h>

#undef DEBUG

static constexpr auto WIDTH = 1600;
static constexpr auto HEIGHT = 900;

enum class Direction { Left, Right, Up, Down };

[[nodiscard]] static float rng() {
    std::random_device dev;
    std::mt19937 rng(dev());
    auto num = rng() / static_cast<float>(rng.max());
    assert(num <= 1.0f);
    assert(num >= 0.0f);
    return num;
}

class Particle {
    Vector2 m_pos;
    Vector2 m_vel { 0, 0 };
    Vector2 m_acc { 0, 0 };
    Vector2 m_gravity { 0, 1000 };
    const Color m_color;
    const float m_radius;

public:
    Particle(Vector2 pos, Vector2 vel, Color color, float radius)
        : m_pos(pos)
        , m_vel(vel)
        , m_color(color)
        , m_radius(radius)
    { }

    static Particle random(Vector2 pos) {

        std::array colors {
            LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, RED, MAROON,
            BEIGE, BROWN, DARKBROWN,
        };

        // std::array colors {
        //     LIGHTGRAY, GRAY, DARKGRAY,
        //     SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET,
        //     DARKPURPLE
        // };

        // size_t idx = rng() * colors.size();
        size_t idx = (fmod(GetTime(), 1)) * colors.size();
        float vel_mul = 0;
        Vector2 vel = { rng()*vel_mul, rng()*vel_mul };
        // float max_rad = 10, min_rad = 10;
        // float rad = std::clamp(rng()*max_rad, min_rad, max_rad);
        float rad = 20;

        return Particle(pos, vel, colors[idx], rad);
    }

    void update(float dt) {
        m_pos = Vector2Add(m_pos, Vector2Scale(m_vel, dt));
        // m_vel = Vector2Add(m_vel, Vector2Scale(m_acc, dt));
        m_vel = Vector2Add(m_vel, Vector2Scale(m_gravity, dt));
    }

    void draw() const {
        DrawCircleV(m_pos, m_radius, m_color);

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

    void resolve_collisions_others(std::span<Particle> others, float dt) {

        Vector2 step = Vector2Scale(m_vel, dt);

        for (auto &other : others) {
            if (&other == this) return;

            auto axis = Vector2Subtract(other.m_pos, m_pos);
            auto axis_norm = Vector2Normalize(axis);
            auto other_step = Vector2Scale(other.m_vel, dt);
            float dist = Vector2Length(axis);

            // DrawLineEx(m_pos, Vector2Add(m_pos, axis), 1, DARKGRAY);

            float steps = Vector2Length(step) + Vector2Length(other_step);
            float diff = dist - steps - m_radius - other.m_radius;
            float delta = abs(diff) / 2.0f;

            if (diff < 0) {
                // move the particle away along the negative collision axis
                m_pos = Vector2Add(m_pos, Vector2Scale(Vector2Negate(axis_norm), delta));
                other.m_pos = Vector2Add(other.m_pos, Vector2Scale(axis_norm, delta));
            }

        }

    }

    void resolve_collisions_wall(float dt) {

        Vector2 step = { m_vel.x * dt, m_vel.y * dt };
        float down  = m_pos.y + m_radius - HEIGHT;
        float up    = m_pos.y - m_radius;
        float right = m_pos.x + m_radius - WIDTH;
        float left  = m_pos.x - m_radius;

        if (up + step.y < 0) {
            m_pos.y = m_radius;
            m_vel.y = 0;
        }

        if (down + step.y > 0) {
            m_pos.y = HEIGHT - m_radius;
            m_vel.y = 0;
        }

        if (left + step.x < 0) {
            m_pos.x = m_radius;
            m_vel.x = 0;
        }

        if (right + step.x > 0) {
            m_pos.x = WIDTH - m_radius;
            m_vel.x = 0;
        }

    }

    void resolve_collisions_container(float dt, Vector2 center, float radius) {

        auto axis = Vector2Subtract(center, m_pos);
        float step = Vector2Length(m_vel)*dt;

        // DrawLineEx(m_pos, Vector2Add(m_pos, axis), 1, DARKGRAY);

        auto diff = Vector2Length(axis) + m_radius + step - radius;
        if (diff > 0) {
            m_pos = Vector2Add(m_pos, Vector2Scale(Vector2Normalize((axis)), diff));
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
    // SetTargetFPS(180);
    InitWindow(WIDTH, HEIGHT, "particles");

    std::vector<Particle> particles;

    float vel = 500;
    float rad = 150;
    // particles.push_back(Particle({ 300, HEIGHT/2.0f }, { vel, 0 }, RED, rad));
    // particles.push_back(Particle({ WIDTH-100, HEIGHT/2.0f }, { -vel, 0 }, BLUE, rad));

    float fut = 0;
    float delay = 0.05;


    Vector2 center = { WIDTH/2.0f, HEIGHT/2.0f };
    float radius = 400;

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(BLACK);
            // DrawFPS(0, 0);
            // DrawText(std::format("particles: {}", particles.size()).c_str(), 0, 20, 20, WHITE);

            if (GetTime() > fut) {
                fut = GetTime() + delay;

                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                    particles.push_back(Particle::random(GetMousePosition()));

                if (IsKeyDown(KEY_J))
                    particles.push_back(Particle::random({ std::clamp(rng()*WIDTH, 100.0f, WIDTH-100.0f), HEIGHT/2.0f }));
            }

            float dt = GetFrameTime();
            int sub_steps = 20;
            float sub_dt = dt / sub_steps;

            for (int i=0; i < sub_steps; ++i) {
                for (auto &p : particles) {
                    p.resolve_collisions_others(particles, sub_dt);
                    p.resolve_collisions_container(sub_dt, center, radius);
                    p.update(sub_dt);
                }
            }

            for (auto &p : particles) {
                p.draw();
            }

            DrawCircleLinesV(center, radius, DARKGRAY);

        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
