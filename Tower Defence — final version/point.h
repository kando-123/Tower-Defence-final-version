#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

const float SOURCE_RADIUS = 20.f;
const float VERTEX_RADIUS = 15.f;
const float TOWER_RADIUS = 25.f;
const float LINE_THICKNESS = 2.f;
const float RECTANGLE_THICKNESS = 5.f;
const sf::Color SOURCE_FILL(0xe0, 0x30, 0x30);
const sf::Color SOURCE_OUTLINE(0xc0, 0x10, 0x10);
const sf::Color VERTEX_FILL(0x80, 0x80, 0x80);
const sf::Color VERTEX_OUTLINE(0x60, 0x60, 0x60);
const sf::Color TOWER_FILL(0x30, 0x30, 0xe0);
const sf::Color TOWER_OUTLINE(0x10, 0x10, 0xc0);

enum class PointType { Source, Vertex, Tower };

class Point
{
private:

	PointType m_type;
	sf::Vector2f m_position;
	sf::CircleShape m_circle;
	std::vector <std::pair<int, sf::RectangleShape>> m_neighbours;

public:

	Point(PointType type, const sf::Vector2f& position);
	
	void addNeighbour(int index, const sf::Vector2f& coords);
	int randomNeighbour();

	PointType getType() const;
	sf::Vector2f getPosition() const;

	void drawLines(sf::RenderWindow& window);
	void drawCircle(sf::RenderWindow& window);
};

