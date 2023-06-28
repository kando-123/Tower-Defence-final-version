#include "defence.h"
#include "error.h"
#include "manager.h"
#include <cmath>

#include <iostream>

void Defence::setRadius(float radius)
{
	m_radius = radius;
}

void Defence::setPeriod(int period)
{
	m_period = period;
	m_counter = std::rand() % period;
}

void Defence::setForce(int force)
{
	m_force = force;
}

void Defence::setHitsPerOnce(int hits)
{
	m_hits_per_once = hits;
}

void Defence::setCost(int cost)
{
	m_cost = cost;
}

void Defence::setTexture(const sf::Texture& texture)
{
	m_sprite.setTexture(texture);
}

void Defence::setPosition(const sf::Vector2f& coords)
{
	m_position = coords;
	m_sprite.setPosition(coords + m_shift);
}

void Defence::setScale(float scale)
{
	m_sprite.setScale(scale, scale);
}

void Defence::setShift(const sf::Vector2f& offset)
{
	m_shift = offset;
}

int Defence::getCost()
{
	return m_cost;
}

float Defence::getRadius()
{
	return m_radius;
}

void Defence::drawYourself(sf::RenderWindow& window)
{
	window.draw(m_sprite);
}

void Defence::tick()
{
	if (++m_counter >= m_period)
		m_counter = 0;
}

bool Defence::ready()
{
	return m_counter == 0 and m_hits_done < m_hits_per_once;
}

void Defence::reset()
{
	m_hits_done = 0;
}

void Shooter::attack(std::list<Entity>::iterator first, std::list<Entity>::iterator last)
{
	while (first != last)
	{
		if (m_hits_done == m_hits_per_once)
			break;
		sf::Vector2f position = first->getPosition();
		float distance = std::hypot(m_position.x - position.x, m_position.y - position.y);
		if (distance < m_radius)
		{
			first->takeHit(m_force);
			++m_hits_done;
		}
		++first;
	}
}

void Freezer::attack(std::list<Entity>::iterator first, std::list<Entity>::iterator last)
{
	while (first != last)
	{
		if (m_hits_done == m_hits_per_once)
			break;
		const sf::Vector2f& position = first->getPosition();
		float distance = std::hypot(m_position.x - position.x, m_position.y - position.y);
		if (not first->isFrozen() and distance < m_radius)
		{
			first->freeze(m_force);
			++m_hits_done;
		}
		++first;
	}
}
