#pragma once
#include "entity.h"
#include <list>
#include <mutex>
#include <filesystem>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

const int DEFENCES_NUMBER = 4;
const std::string LABELS[DEFENCES_NUMBER]{ "unishooter", "multishooter", "cannon", "freezer" };
enum class DefenceType { UniShooter, MultiShooter, Cannon, Freezer, None = -1 };

class Defence
{
protected:

	float m_radius = 0.f;
	int m_period = 1;
	int m_counter = 0;
	int m_force = 0;
	int m_hits_per_once = 0;
	int m_hits_done = 0;
	int m_cost = 0;
	sf::Vector2f m_position;
	sf::Vector2f m_shift;
	sf::Sprite m_sprite;

public:

	Defence() = default;

	void setRadius(float radius);
	void setPeriod(int period);
	void setForce(int force);
	void setHitsPerOnce(int hits);
	void setCost(int cost);
	void setTexture(const sf::Texture& texture);
	void setPosition(const sf::Vector2f& coords);
	void setScale(float scale);
	void setShift(const sf::Vector2f& offset);

	int getCost();
	float getRadius();

	void drawYourself(sf::RenderWindow& window);

	void tick();
	bool ready();
	virtual void attack(std::list<Entity>::iterator first, std::list<Entity>::iterator last) = 0;
	virtual void reset();
};

class Shooter : public Defence
{
public:

	void attack(std::list<Entity>::iterator first, std::list<Entity>::iterator last) override;
};

class Freezer : public Defence
{
public:

	void attack(std::list<Entity>::iterator first, std::list<Entity>::iterator last) override;
};
