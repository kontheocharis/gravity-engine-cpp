#include <SFML/Graphics.hpp>
#include <eigen3/Eigen/Dense>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <iostream>
#include <random>

namespace eig = Eigen;


struct Particle
{
    double mass;
    double radius;

    eig::Vector2d position;
    eig::Vector2d velocity;
    eig::Vector2d acceleration = eig::Vector2d::Zero();
};

class ParticleManager
{
    std::shared_ptr<std::vector<Particle>> particles;
    double big_g;
    double time_multiplier;
    bool wrapping;
    int window_width, window_height;
    double collision_dampening_coefficient;

    void wrap_particle(Particle& particle) const 
    {
        eig::Vector2d unit_normal(0, 0);
        bool need_to_wrap = false;

        if (particle.position.x() > window_width)
        {
            need_to_wrap = true;
            unit_normal += eig::Vector2d { -1, 0 };
            particle.position.x() = window_width;
        } 
        else if (particle.position.x() < 0)
        {
            need_to_wrap = true;
            unit_normal += eig::Vector2d { 1, 0 };
            particle.position.x() = 0;
        }

        if (particle.position.y() > window_height)
        {
            need_to_wrap = true;
            unit_normal += eig::Vector2d { 0, -1 };
            particle.position.y() = window_height;
        } 
        else if (particle.position.y() < 0)
        {
            need_to_wrap = true;
            unit_normal += eig::Vector2d { 0, 1 };
            particle.position.y() = 0;
        }

        if (!need_to_wrap) return;

        unit_normal.normalize();
        particle.velocity = particle.velocity - 2 * (particle.velocity.dot(unit_normal)) * unit_normal;
    }

     bool are_touching(const Particle& p1, const Particle& p2) const
     {
         const double epsilon = 1.0;
         return (p2.position - p1.position).norm() <= p1.radius + p2.radius + epsilon;
     }

public:
    ParticleManager(
        std::shared_ptr<std::vector<Particle>> particles,
        double big_g,
        bool wrapping,
        int window_width, int window_height
        ) : 
        particles(particles),
        big_g(big_g),
        wrapping(wrapping),
        window_width(window_width), window_height(window_height) {  }

    void calculate_physics(const sf::Time& delta_time)
    {
        if (particles->size() <= 1) return;

        std::vector<std::pair<int, int>> collision_checked = {};

        for (int i = 0; i < particles->size(); ++i) 
        {
            auto& particle = particles->operator[](i);

            particle.position += 
                particle.velocity * delta_time.asSeconds() +
                0.5 * particle.acceleration * delta_time.asSeconds() * delta_time.asSeconds();

            const eig::Vector2d last_acceleration = particle.acceleration;

            eig::Vector2d new_acceleration = eig::Vector2d::Zero();

            // Loop over all particles except the current one
            for (int j = 0; j < particles->size(); ++j) 
            {
                if (i == j) continue;
                auto& p = particles->operator[](j);
                new_acceleration += (p.position - particle.position).normalized() * p.mass * big_g / (p.position - particle.position).squaredNorm();
            }

            particle.acceleration = new_acceleration;
            particle.velocity += delta_time.asSeconds() * (new_acceleration + last_acceleration) * 0.5;

            // Check collisions
            for (int j = 0; j < particles->size(); ++j) 
            {
                if (i == j) continue;
                auto& p = particles->operator[](j);

                if (!are_touching(particle, p)) continue;
                if (std::find(collision_checked.begin(), collision_checked.end(), std::pair<int,int>(std::minmax(i,j))) != collision_checked.end()) continue;

                const auto distance = (p.position - particle.position).norm();
                const auto unit = (p.position - particle.position) / distance;

                const auto moment = 2 * (particle.velocity.dot(unit) - p.velocity.dot(unit)) / (particle.mass + p.mass);

                particle.velocity = particle.velocity - moment * p.mass * unit;
                p.velocity = p.velocity + moment * particle.mass * unit;
                collision_checked.push_back(std::pair<int, int>(std::minmax(i,j)));
            }

            if (wrapping) wrap_particle(particle);
        }

    }

    void create_circular_particle(int pos_x, int pos_y, double radius, double density)
    {
        const auto& sun = particles->operator[](0);
        const eig::Vector2d position(pos_x, pos_y);
        const auto to_sun = (sun.position - position);

        particles->push_back({
            4/3 * M_PI * pow(radius, 3) * density,
            radius,
            position,
            eig::Rotation2Dd(M_PI/2).toRotationMatrix() * to_sun.normalized() * sqrt(big_g * sun.mass / to_sun.norm())
        });
    }

};

class ParticleRenderer
{
    std::shared_ptr<std::vector<Particle>> particles;
    sf::Color particle_color;

public:
    ParticleRenderer(
        std::shared_ptr<std::vector<Particle>> particles,
        sf::Color particle_color
        ) : 
        particles(particles),
        particle_color(particle_color) {  }

    std::vector<sf::CircleShape> create_particle_shapes() const
    {
        auto create_circle = [&](Particle& p) {
            auto circle = sf::CircleShape(p.radius);
            circle.setFillColor(particle_color);
            circle.setOrigin(p.radius, p.radius);
            circle.setPosition(p.position.x(), p.position.y());
            return circle;
        };

        std::vector<sf::CircleShape> shapes;
        std::transform(particles->begin(), particles->end(), std::back_inserter(shapes), create_circle);
        return shapes;
    }
};

int main(int argc, char *argv[])
{
    const int window_width = sf::VideoMode::getDesktopMode().width;
    const int window_height = sf::VideoMode::getDesktopMode().height;

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "My window");
    window.setVerticalSyncEnabled(true);

    sf::Clock clock;

    const double sun_radius = 75;
    const double sun_density = 10000;
    const double big_g = 2e-3;

    auto particles = std::make_shared<std::vector<Particle>>(std::initializer_list<Particle>{
        Particle { 4/3 * M_PI * pow(sun_radius, 3) * sun_density, sun_radius, { window_width/2, window_height/2 }, { 0, 0 } }
    });

    ParticleManager particle_manager(particles, big_g, false, window_width, window_height);
    ParticleRenderer particle_renderer(particles, sf::Color::White);

    std::random_device rd;
    std::mt19937 seeder(rd());
    std::uniform_int_distribution<> radius_dist(5, 15);

    while (window.isOpen())
    {
        sf::Time delta_time = clock.restart();
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            else if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visible_area(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visible_area));
            }

            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == 0)
            {
                const int pos_x = event.mouseButton.x;
                const int pos_y = event.mouseButton.y;
                particle_manager.create_circular_particle(pos_x, pos_y, radius_dist(seeder), 10000);
            }
        }

        window.clear(sf::Color::Black);

        for (auto& shape : particle_renderer.create_particle_shapes()) { window.draw(shape); }

        particle_manager.calculate_physics(delta_time);

        window.display();
    }

    return 0;
}
