#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>
#include <iostream>
#include <memory>

// ==========================================
// Vector utilities
// ==========================================
struct Vec {
    float x, y;
    Vec() : x(0), y(0) {}
    Vec(float x_, float y_) : x(x_), y(y_) {}
    Vec operator+(const Vec& o) const { return Vec(x+o.x, y+o.y); }
    Vec operator-(const Vec& o) const { return Vec(x-o.x, y-o.y); }
    Vec operator*(float s) const { return Vec(x*s, y*s); }
    Vec operator/(float s) const { return Vec(x/s, y/s); }
};

inline float dot(const Vec& a, const Vec& b) { return a.x*b.x + a.y*b.y; }
inline float len(const Vec& a) { return std::sqrt(a.x*a.x + a.y*a.y); }
inline Vec normalize(const Vec& a) { float L = len(a); return (L==0? Vec(0,0) : a / L); }

// ==========================================
// Ball
// ==========================================
struct Ball {
    Vec pos;
    Vec vel;
    float radius;
    float mass;
    sf::Color color;
    Ball(const Vec& p, const Vec& v, float r, float m, sf::Color c)
        : pos(p), vel(v), radius(r), mass(m), color(c) {}
};

// ==========================================
// Random
// ==========================================
std::mt19937 rng((unsigned)std::random_device{}());
std::uniform_real_distribution<float> uni01(0.f, 1.f);

sf::Color randomColor() {
    static std::vector<sf::Color> colors = {
        sf::Color(52, 152, 219),   // Biru
        sf::Color(243, 156, 18),   // Orange
        sf::Color(241, 196, 15)    // Kuning
    };
    return colors[rng() % colors.size()];
}

// ==========================================
// Collision response
// ==========================================
void resolveCollision(Ball &a, Ball &b) {
    Vec n = a.pos - b.pos;
    float dist = len(n);
    if (dist == 0.f) {
        n = Vec(0.01f, 0.01f);
        dist = len(n);
    }
    float penetration = a.radius + b.radius - dist;
    if (penetration > 0) {
        Vec n_norm = n / dist;
        float totalMass = a.mass + b.mass;
        float correction = 0.5f;
        a.pos = a.pos + n_norm * (penetration * (b.mass / totalMass) * correction);
        b.pos = b.pos - n_norm * (penetration * (a.mass / totalMass) * correction);

        Vec rv = a.vel - b.vel;
        Vec normal = n_norm;
        float velAlongNormal = dot(rv, normal);
        if (velAlongNormal > 0) return;

        float e = 1.0f;
        float j = -(1 + e) * velAlongNormal;
        j /= (1.0f / a.mass) + (1.0f / b.mass);

        Vec impulse = normal * j;
        a.vel = a.vel + impulse * (1.0f / a.mass);
        b.vel = b.vel - impulse * (1.0f / b.mass);
    }
}

// ==========================================
// Quadtree structure
// ==========================================
struct AABB {
    float x, y, w, h;
    bool contains(const Ball* b) const {
        return (b->pos.x >= x && b->pos.x <= x+w &&
                b->pos.y >= y && b->pos.y <= y+h);
    }
    bool intersects(const AABB &o) const {
        return !(o.x > x + w || o.x + o.w < x || o.y > y + h || o.y + o.h < y);
    }
};

struct Quadtree {
    AABB box;
    int capacity;
    std::vector<Ball*> points;
    bool divided = false;
    std::unique_ptr<Quadtree> nw, ne, sw, se;

    Quadtree(const AABB &b, int cap=6) : box(b), capacity(cap) {}

    void subdivide() {
        float hw = box.w/2, hh = box.h/2;
        nw = std::make_unique<Quadtree>(AABB{box.x, box.y, hw, hh}, capacity);
        ne = std::make_unique<Quadtree>(AABB{box.x+hw, box.y, hw, hh}, capacity);
        sw = std::make_unique<Quadtree>(AABB{box.x, box.y+hh, hw, hh}, capacity);
        se = std::make_unique<Quadtree>(AABB{box.x+hw, box.y+hh, hw, hh}, capacity);
        divided = true;
    }

    bool insert(Ball* b) {
        if (!box.contains(b)) return false;
        if ((int)points.size() < capacity) {
            points.push_back(b);
            return true;
        }
        if (!divided) subdivide();
        if (nw->insert(b)) return true;
        if (ne->insert(b)) return true;
        if (sw->insert(b)) return true;
        if (se->insert(b)) return true;
        return false;
    }

    void query(const AABB &range, std::vector<Ball*> &out) {
        if (!box.intersects(range)) return;
        for (auto *b : points)
            if (range.contains(b)) out.push_back(b);
        if (!divided) return;
        nw->query(range,out);
        ne->query(range,out);
        sw->query(range,out);
        se->query(range,out);
    }
};

// ==========================================
// Main Simulation
// ==========================================
int main() {
    const unsigned int W = 1000;
    const unsigned int H = 700;

    sf::RenderWindow window(sf::VideoMode(W,H), "Collision Simulation");
    window.setFramerateLimit(120);

    std::vector<Ball> balls;

    std::uniform_real_distribution<float> rx(50.f, W-50.f);
    std::uniform_real_distribution<float> ry(50.f, H-50.f);
    std::uniform_real_distribution<float> rv(-120.f, 120.f);
    std::uniform_real_distribution<float> rr(10.f, 28.f);
    std::uniform_real_distribution<float> rm(0.5f, 3.f);

    for (int i=0;i<40;i++) {
        float r = 12.f;       
        float m = 1.0f;          
        float speedMultiplier = 1.2f;
        balls.emplace_back(
            Vec(rx(rng), ry(rng)),
            Vec(rv(rng)*speedMultiplier, rv(rng)*speedMultiplier),
            r, m, randomColor()
        );
    }

    bool paused = false;
    bool useQuadtree = false;
    sf::Clock clock;
    float accumulator = 0.f;
    const float dt = 1.f / 120.f;

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
            else if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Escape) window.close();
                if (ev.key.code == sf::Keyboard::Space) paused = !paused;
                if (ev.key.code == sf::Keyboard::Q) useQuadtree = !useQuadtree;
            }
        }

        float frameTime = clock.restart().asSeconds();
        if (frameTime > 0.25f) frameTime = 0.25f;
        accumulator += frameTime;

        while (accumulator >= dt) {
            accumulator -= dt;
            if (!paused) {
                for (auto &b : balls) {
                    b.pos.x += b.vel.x * dt;
                    b.pos.y += b.vel.y * dt;
                }

                for (auto &b : balls) {
                    if (b.pos.x - b.radius < 0) { b.pos.x = b.radius; b.vel.x *= -1; }
                    if (b.pos.x + b.radius > W) { b.pos.x = W - b.radius; b.vel.x *= -1; }
                    if (b.pos.y - b.radius < 0) { b.pos.y = b.radius; b.vel.y *= -1; }
                    if (b.pos.y + b.radius > H) { b.pos.y = H - b.radius; b.vel.y *= -1; }
                }

                if (!useQuadtree) {
                    // BRUTE FORCE =========================================
                    size_t n = balls.size();
                    for (size_t i=0;i<n;i++) {
                        for (size_t j=i+1;j<n;j++) {
                            float dx = std::abs(balls[i].pos.x - balls[j].pos.x);
                            float dy = std::abs(balls[i].pos.y - balls[j].pos.y);
                            if (dx > balls[i].radius + balls[j].radius) continue;
                            if (dy > balls[i].radius + balls[j].radius) continue;
                            resolveCollision(balls[i], balls[j]);
                        }
                    }
                } else {
                    // QUADTREE ============================================
                    Quadtree qt(AABB{0,0,(float)W,(float)H}, 6);
                    for (auto &b : balls) qt.insert(&b);
                    for (size_t i=0;i<balls.size(); i++) {
                        Ball &a = balls[i];
                        float r = a.radius + 30;
                        AABB range{a.pos.x-r, a.pos.y-r, r*2, r*2};
                        std::vector<Ball*> candidates;
                        qt.query(range, candidates);
                        for (auto *bptr : candidates) {
                            if (bptr == &a) continue;
                            if (bptr < &a) continue;
                            float dx = std::abs(a.pos.x - bptr->pos.x);
                            float dy = std::abs(a.pos.y - bptr->pos.y);
                            if (dx > a.radius + bptr->radius) continue;
                            if (dy > a.radius + bptr->radius) continue;
                            resolveCollision(a, *bptr);
                        }
                    }
                }
            }
        }

        window.clear(sf::Color(30,30,30));
        for (auto &b : balls) {
            sf::CircleShape shape(b.radius);
            shape.setOrigin(b.radius, b.radius);
            shape.setPosition(b.pos.x, b.pos.y);
            shape.setFillColor(b.color);
            window.draw(shape);
        }

        sf::Font font;
        if (font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
            sf::Text info;
            info.setFont(font);
            info.setCharacterSize(14);
            info.setFillColor(sf::Color::White);
            info.setPosition(8,8);
            std::string s = "Balls: " + std::to_string(balls.size()) +
                "  | Space: pause  | Q: toggle quadtree (" + (useQuadtree?"ON":"OFF") + ")";
            info.setString(s);
            window.draw(info);
        }

        window.display();
    }

    return 0;
}
