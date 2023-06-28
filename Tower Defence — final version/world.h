#pragma once
#include "point.h"
#include "graph.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

class World
{
private:

	sf::Vector2f m_dimensions;
	std::vector<Point> m_points;
	int m_sources_number = 0;
	
	World() = default;

public:

	static World& getInstance()
	{
		static World instance;
		return instance;
	}
	
	void setDimensions(float width, float height);
	sf::Vector2f getDimensions() const;

	void loadMap(Graph& graph);

	void drawEverything(sf::RenderWindow& window);

	int getRandomSource();
	int getRandomNeighbour(int index);
	PointType getType(int index);
	sf::Vector2f getCoords(int index);
};

