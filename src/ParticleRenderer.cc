#include "ParticleRenderer.hh"

std::vector<sf::CircleShape> ParticleRenderer::create_particle_shapes() const
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
