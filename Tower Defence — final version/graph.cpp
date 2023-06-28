#include "graph.h"

Element::Element(PointType a_type, const sf::Vector2f a_position)
	: type(a_type), position(a_position)
{
	visited = false;
}
