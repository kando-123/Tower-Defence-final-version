#include "entity.h"
#include "manager.h"
#include "world.h"

Entity::Entity(int type) : m_type(type)
{
	Manager& manager_ref = Manager::getInstance();
	const EntityRecord& record = manager_ref.getEntityRecord(m_type);
	m_health = record.health;
	m_speed = record.speed;
	m_shift = .5f * record.scale * record.dimensions;
	m_freeze_count = 0;

	World& world_ref = World::getInstance();
	int origin = world_ref.getRandomSource();
	m_position = world_ref.getCoords(origin);
	m_target = world_ref.getRandomNeighbour(origin);
	sf::Vector2f there = world_ref.getCoords(m_target);
	float hypotenuse = std::hypot(there.x - m_position.x, there.y - m_position.y);
	m_step = m_speed / hypotenuse * (there - m_position);
	m_steps_count = hypotenuse / m_speed;
	
	m_sprite.setTexture(record.texture);
	m_sprite.setScale(record.scale, record.scale);
	m_sprite.setPosition(m_position - m_shift);
}

void Entity::drawYourself(sf::RenderWindow& window)
{
	window.draw(m_sprite);
}

bool Entity::move()
{
	if (m_freeze_count > 0)
	{
		--m_freeze_count;
		return true;
	}
	if (--m_steps_count == 0)
	{
		World& world_ref = World::getInstance();
		m_position = world_ref.getCoords(m_target);
		m_sprite.setPosition(m_position - m_shift);
		if (world_ref.getType(m_target) == PointType::Tower)
			return false;
		else
		{
			m_target = world_ref.getRandomNeighbour(m_target);
			sf::Vector2f there = world_ref.getCoords(m_target);
			float hypotenuse = std::hypot(there.x - m_position.x, there.y - m_position.y);
			m_step = m_speed / hypotenuse * (there - m_position);
			m_steps_count = hypotenuse / m_speed;
		}
	}
	else
	{
		m_position += m_step;
		m_sprite.move(m_step);
	}
	return true;
}

int Entity::getType()
{
	return m_type;
}

sf::Vector2f Entity::getPosition()
{
	return m_position;
}

void Entity::takeHit(int force)
{
	m_health -= force;
}

void Entity::freeze(int force)
{
	m_freeze_count += force;
}

bool Entity::isFrozen()
{
	return m_freeze_count > 0;
}

bool Entity::isAlive()
{
	return m_health > 0;
}

