#include "error.h"
#include "point.h"
#include <cmath>
#include <cstdlib>
#include <numbers>

Point::Point(PointType type, const sf::Vector2f& position)
	: m_type(type), m_position(position)
{
    switch (type)
    {
    case PointType::Source:
        m_circle.setRadius(SOURCE_RADIUS);
        m_circle.setPosition(position.x - SOURCE_RADIUS, position.y - SOURCE_RADIUS);
        m_circle.setFillColor(SOURCE_FILL);
        m_circle.setOutlineColor(SOURCE_OUTLINE);
        break;
    case PointType::Vertex:
        m_circle.setRadius(VERTEX_RADIUS);
        m_circle.setPosition(position.x - VERTEX_RADIUS, position.y - VERTEX_RADIUS);
        m_circle.setFillColor(VERTEX_FILL);
        m_circle.setOutlineColor(VERTEX_OUTLINE);
        break;
    case PointType::Tower:
        m_circle.setRadius(TOWER_RADIUS);
        m_circle.setPosition(position.x - TOWER_RADIUS, position.y - TOWER_RADIUS);
        m_circle.setFillColor(TOWER_FILL);
        m_circle.setOutlineColor(TOWER_OUTLINE);
        break;
    }
    m_circle.setOutlineThickness(LINE_THICKNESS);
}

void Point::addNeighbour(int index, const sf::Vector2f& coords)
{
    sf::Vector2f dimensions;
    dimensions.x = std::hypot(coords.x - m_position.x, coords.y - m_position.y);
    dimensions.y = RECTANGLE_THICKNESS;
    sf::RectangleShape rectangle(dimensions);
    rectangle.setFillColor(VERTEX_FILL);
    rectangle.setOutlineColor(VERTEX_OUTLINE);
    rectangle.setOutlineThickness(LINE_THICKNESS);
    rectangle.setPosition(m_position.x, m_position.y);
    rectangle.setOrigin(0.f, .5f * RECTANGLE_THICKNESS);
    float angle = std::atan2(coords.y - m_position.y, coords.x - m_position.x);
    angle *= 180.f * std::numbers::inv_pi_v<float>;
    rectangle.rotate(angle);
    m_neighbours.emplace_back(index, rectangle);
}

int Point::randomNeighbour()
{
    if (m_neighbours.empty())
        throw Error(Problem::OutOfRange);
    return m_neighbours[rand() % m_neighbours.size()].first;
}

PointType Point::getType() const
{
    return m_type;
}

sf::Vector2f Point::getPosition() const
{
    return m_position;
}

void Point::drawLines(sf::RenderWindow& window)
{
    for (auto it = m_neighbours.begin(); it != m_neighbours.end(); ++it)
        window.draw(it->second);
}

void Point::drawCircle(sf::RenderWindow& window)
{
    window.draw(m_circle);
}

