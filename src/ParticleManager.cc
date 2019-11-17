#include "ParticleManager.hh"

#include <eigen3/Eigen/Dense>
#include <memory>
#include <SFML/Graphics.hpp>

#include "Particle.hh"

namespace eig = Eigen;

void ParticleManager::wrap_particle(Particle& particle) const
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

bool ParticleManager::are_touching(const Particle& p1, const Particle& p2) const
{
    const double epsilon = 1.0;
    return (p2.position - p1.position).norm() <= p1.radius + p2.radius + epsilon;
}

void ParticleManager::calculate_physics(const sf::Time& delta_time)
{
    if (particles->size() <= 1) return;

    std::vector<std::pair<int, int>> collision_checked = {};

    for (int i = 0; i < particles->size(); ++i) 
    {
        auto& particle = particles->at(i);

        particle.position += 
            particle.velocity * delta_time.asSeconds() +
            0.5 * particle.acceleration * delta_time.asSeconds() * delta_time.asSeconds();

        const eig::Vector2d last_acceleration = particle.acceleration;

        eig::Vector2d new_acceleration = eig::Vector2d::Zero();

        // Loop over all particles except the current one
        for (int j = 0; j < particles->size(); ++j) 
        {
            if (i == j) continue;
            auto& p = particles->at(j);
            new_acceleration += (p.position - particle.position).normalized() * p.mass * big_g / (p.position - particle.position).squaredNorm();
        }

        particle.acceleration = new_acceleration;
        particle.velocity += delta_time.asSeconds() * (new_acceleration + last_acceleration) * 0.5;

        // Check collisions
        for (int j = 0; j < particles->size(); ++j) 
        {
            if (i == j) continue;
            auto& p = particles->at(j);

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

void ParticleManager::create_circular_particle(int pos_x, int pos_y, double radius, double density)
{
    const auto& sun = particles->at(0);
    const eig::Vector2d position(pos_x, pos_y);
    const auto to_sun = (sun.position - position);

    particles->push_back({
        4/3 * M_PI * pow(radius, 3) * density,
        radius,
        position,
        eig::Rotation2Dd(M_PI/2).toRotationMatrix() * to_sun.normalized() * sqrt(big_g * sun.mass / to_sun.norm())
    });
}
