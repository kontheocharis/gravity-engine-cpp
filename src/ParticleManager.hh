#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

#include "Common.hh"
#include "Particle.hh"

class ParticleManager
{
    const double epsilon = 1.0;

    std::shared_ptr<std::vector<Particle>> particles;
    double big_g;
    double time_multiplier;
    bool wrapping;
    int window_width, window_height;
    double collision_dampening_coefficient;

    void wrap_particle(Particle& particle);
    void calculate_collisions(std::vector<Particle>::iterator);

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

    void calculate_physics(const sf::Time& delta_time);
    void create_circular_particle(int pos_x, int pos_y, double radius, double density);
};
