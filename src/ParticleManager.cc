#include "ParticleManager.hh"

#include <memory>
#include <SFML/Graphics.hpp>
#include <iostream>

#include "Common.hh"
#include "Particle.hh"


void ParticleManager::wrap_particle(Particle& particle)
{
    eig::Vector2d unit_normal { 0, 0 };
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
    particle.velocity = cor * (particle.velocity - 2 * (particle.velocity.dot(unit_normal)) * unit_normal);
}


void ParticleManager::calculate_physics(const sf::Time& delta_time, bool has_gravity)
{
    if (particles->size() <= 1) return;

    const double dt = delta_time.asSeconds();

    for (auto i = particles->begin(); i != particles->end(); ++i) 
    {
        calculate_collisions(i);

        i->position += i->velocity * dt + 0.5 * i->acceleration * dt * dt;

        if (has_gravity)
        {
            const auto last_acceleration = i->acceleration;

            eig::Vector2d new_acceleration { 0, 0 };

            // Loop over all particles
            for (auto j = particles->begin(); j != particles->end(); ++j) 
            {
                if (j == i) continue;

                new_acceleration += (j->position - i->position).normalized() *
                    (j->mass * big_g) / (j->position - i->position).squaredNorm();
            }

            i->acceleration = new_acceleration;
            i->velocity += dt * (new_acceleration + last_acceleration) * 0.5;
        }


        if (wrapping) wrap_particle(*i);
    }
}


void ParticleManager::calculate_collisions(std::vector<Particle>::iterator i)
{
    // Check collisions of all particles after the current one
    for (auto j = std::next(i); j != particles->end(); ++j) 
    {
        const auto distance = (j->position - i->position).norm();
        const auto alpha = distance - (i->radius + j->radius + epsilon);

        if (alpha > 0) continue;

        const auto unit = (j->position - i->position) / distance;
        const auto moment = 2 * (i->velocity.dot(unit) - j->velocity.dot(unit)) / (i->mass + j->mass);

        i->velocity = i->velocity - moment * j->mass * unit;
        j->velocity = j->velocity + moment * i->mass * unit;

        if (alpha < 0)
        {
            std::vector<Particle>::iterator small_particle, large_particle;

            if (j->mass < i->mass) { small_particle = j; large_particle = i; }
            else { small_particle = i; large_particle = j; }

            small_particle->position -= (small_particle->position - large_particle->position) / distance * (alpha);
        }
    }
}


void ParticleManager::create_circular_particle(int pos_x, int pos_y, double radius, double density)
{
    const auto& sun = particles->at(0);
    const eig::Vector2d position { pos_x, pos_y };
    const auto to_sun = (sun.position - position);

    particles->push_back({
        4/3 * M_PI * pow(radius, 3) * density,
        radius,
        position,
        eig::Rotation2Dd(M_PI/2).toRotationMatrix() * to_sun.normalized() * sqrt(big_g * sun.mass / to_sun.norm())
    });
}
