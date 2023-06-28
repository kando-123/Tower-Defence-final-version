#pragma once
#include "point.h"
#include <vector>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

struct Element
{
	std::vector<int> neighbours, antineighbours;
	sf::Vector2f position;
	bool visited;
	PointType type;
	
	Element(PointType type, const sf::Vector2f position);
};

struct Graph
{
	int sources_count = 0;
	int towers_count = 0;
	std::vector<Element> body;
};

