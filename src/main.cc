#include <SFML/Graphics.hpp>
#include <eigen3/Eigen/Dense>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <iostream>
#include <random>

#include "Particle.hh"
#include "ParticleManager.hh"
#include "ParticleRenderer.hh"

namespace eig = Eigen;

int main(int argc, char *argv[])
{
    const int window_width = sf::VideoMode::getDesktopMode().width;
    const int window_height = sf::VideoMode::getDesktopMode().height;

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Gravity Engine++");
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
