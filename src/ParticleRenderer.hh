#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "Particle.hh"

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

    std::vector<sf::CircleShape> create_particle_shapes() const;
};
