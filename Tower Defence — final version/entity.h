#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

class Entity
{
private:

	int m_type;
	int m_health;
	float m_speed;
	sf::Sprite m_sprite;
	sf::Vector2f m_shift;
	sf::Vector2f m_position;
	sf::Vector2f m_step;
	int m_target;
	int m_steps_count;
	int m_freeze_count;

public:

	Entity(int type);
	
	void drawYourself(sf::RenderWindow& window);
	bool move();

	int getType();
	sf::Vector2f getPosition();

	void takeHit(int force);
	void freeze(int force);
	bool isFrozen();
	bool isAlive();
};

