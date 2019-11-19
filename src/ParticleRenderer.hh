#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "Particle.hh"

class ParticleRenderer
{
    sf::Color particle_color;

public:
    ParticleRenderer(sf::Color particle_color) : particle_color(particle_color) {}

    std::vector<sf::CircleShape> create_particle_shapes(const std::vector<Particle>&) const;
};
