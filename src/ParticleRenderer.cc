#include "ParticleRenderer.hh"

std::vector<sf::CircleShape> ParticleRenderer::create_particle_shapes(const std::vector<Particle>& particles) const
{
    auto create_circle = [&](const Particle& p) {
        auto circle = sf::CircleShape(p.radius);
        circle.setFillColor(particle_color);
        circle.setOrigin(p.radius, p.radius);
        circle.setPosition(p.position.x(), p.position.y());
        return circle;
    };

    std::vector<sf::CircleShape> particle_shapes;
    std::transform(particles.begin(), particles.end(), std::back_inserter(particle_shapes), create_circle);
    return particle_shapes;
}
