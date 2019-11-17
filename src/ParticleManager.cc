#include "ParticleManager.hh"

#include <eigen3/Eigen/Dense>
#include <memory>
#include <SFML/Graphics.hpp>
#include <iostream>

#include "Particle.hh"

namespace eig = Eigen;

void ParticleManager::wrap_particle(Particle& particle) const
{
    eig::Vector2d unit_normal(0, 0);
    bool need_to_wrap = false;

    if (particle.position.x() + particle.radius > window_width)
    {
        need_to_wrap = true;
        unit_normal += eig::Vector2d { -1, 0 };
        particle.position.x() = window_width - particle.radius;
    } 
    else if (particle.position.x() - particle.radius < 0)
    {
        need_to_wrap = true;
        unit_normal += eig::Vector2d { 1, 0 };
        particle.position.x() = particle.radius;
    }

    if (particle.position.y() + particle.radius > window_height)
    {
        need_to_wrap = true;
        unit_normal += eig::Vector2d { 0, -1 };
        particle.position.y() = window_height - particle.radius;
    } 
    else if (particle.position.y() - particle.radius < 0)
    {
        need_to_wrap = true;
        unit_normal += eig::Vector2d { 0, 1 };
        particle.position.y() = particle.radius;
    }

    if (!need_to_wrap) return;

    unit_normal.normalize();
    particle.velocity = particle.velocity - 2 * (particle.velocity.dot(unit_normal)) * unit_normal;
}

void ParticleManager::calculate_physics(const sf::Time& delta_time)
{
    if (particles->size() <= 1) return;

    const double dt = delta_time.asSeconds();

    for (int i = 0; i < particles->size(); ++i) 
    {
        auto& particle = particles->at(i);

        particle.position += 
            particle.velocity * dt +
            0.5 * particle.acceleration * dt * dt;

        const eig::Vector2d last_acceleration = particle.acceleration;

        eig::Vector2d new_acceleration = eig::Vector2d::Zero();

        // Loop over all particles
        for (int j = 0; j < particles->size(); ++j) 
        {
            if (j == i) continue;
            auto& p = particles->at(j);
            new_acceleration += (p.position - particle.position).normalized() * p.mass * big_g / (p.position - particle.position).squaredNorm();
        }

        particle.acceleration = new_acceleration;
        particle.velocity += dt * (new_acceleration + last_acceleration) * 0.5;

        calculate_collisions(particle, i);

        if (wrapping) wrap_particle(particle);
    }

}

void ParticleManager::calculate_collisions(Particle& particle, int particle_index) const
{
    // Check collisions of all particles after the current one
    for (int j = particle_index + 1; j < particles->size(); ++j) 
    {
        auto& p = particles->at(j);

        const auto distance = (p.position - particle.position).norm();
        if (distance - (particle.radius + p.radius + epsilon) > 0) continue;

        const auto unit = (p.position - particle.position) / distance;

        const auto moment = 2 * (particle.velocity.dot(unit) - p.velocity.dot(unit)) / (particle.mass + p.mass);

        particle.velocity = particle.velocity - moment * p.mass * unit;
        p.velocity = p.velocity + moment * particle.mass * unit;
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
