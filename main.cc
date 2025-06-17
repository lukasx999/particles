#include <cassert>
#include <print>
#include <format>
#include <random>
#include <span>

#include <raylib.h>
#include <raymath.h>

#undef DEBUG
#define NO_GRAVITY

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
    Vector2 m_gravity { 0, 500 };
    const Color m_color;
    static constexpr float m_radius = 50;

public:
    Particle(Vector2 pos, Vector2 vel, Color color)
        : m_pos(pos)
        , m_vel(vel)
        , m_color(color)
    { }

    static Particle random() {
        std::array colors {
            RED, BLUE, GREEN
        };
        size_t idx = rng() * colors.size();
        return Particle({ rng()*WIDTH, rng()*HEIGHT }, { rng(), rng() }, colors[idx]);
    }

    void update(float dt) {
        m_pos = Vector2Add(m_pos, Vector2Scale(m_vel, dt));
        m_vel = Vector2Add(m_vel, Vector2Scale(m_acc, dt));
        #ifndef NO_GRAVITY
        m_vel = Vector2Add(m_vel, Vector2Scale(m_gravity, dt));
        #endif // NO_GRAVITY
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

    void resolve_collisions_others(std::span<Particle> others) {

        for (auto &other : others) {
            if (&other == this) return;

            // float dist = Vector2Distance(m_pos, other.m_pos) / 2;
            auto axis = Vector2Subtract(other.m_pos, m_pos);
            DrawLineEx(m_pos, Vector2Add(m_pos, axis), 1, PURPLE);

            if (CheckCollisionCircles(m_pos, m_radius, other.m_pos, other.m_radius)) {
                // m_pos = Vector2Add(m_pos, Vector2Scale(Vector2Normalize(axis), dist));
                // other.m_pos = Vector2Add(other.m_pos, Vector2Scale(axis, dist));
                m_vel *= -1;
                other.m_vel *= -1;
            }

        }

    }

    void resolve_collisions_wall() {
        assert(m_pos.x < WIDTH);
        assert(m_pos.y < HEIGHT);
        assert(m_pos.y > 0);
        assert(m_pos.x > 0);

        float damp_factor = 0.95;
        bool down  = m_pos.y + m_radius >= HEIGHT;
        bool up    = m_pos.y - m_radius <= 0;
        bool right = m_pos.x + m_radius >= WIDTH;
        bool left  = m_pos.x - m_radius <= 0;

        if (up || down) {
            m_vel.y *= -damp_factor;
        }

        if (right || left) {
            m_vel.x *= -damp_factor;
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

    float vel = 300;

    std::array particles {
        // Particle({ 100, HEIGHT/2.0f }, { vel, 0 }, RED),
        // Particle({ WIDTH-100, HEIGHT/2.0f }, { -vel, 0 }, BLUE),
        Particle::random(),
        Particle::random(),
        Particle::random(),
        Particle::random(),
        Particle::random(),
        Particle::random(),
    };

    // std::vector<Particle> particles;
    // for (int i=0; i < 5; ++i) {
    //     particles.push_back(Particle::random());
    // }

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(BLACK);

            for (auto &p : particles) {

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

                p.resolve_collisions_others(particles);
                p.resolve_collisions_wall();
                p.update(GetFrameTime());
                p.draw();
            }

        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
