#include "world.h"
#include "error.h"
#include "graph.h"
#include "point.h"

void World::setDimensions(float width, float height)
{
    m_dimensions.x = width;
    m_dimensions.y = height;
}

sf::Vector2f World::getDimensions() const
{
    return m_dimensions;
}

void World::loadMap(Graph& graph)
{
    m_sources_number = graph.sources_count;
    for (auto it = graph.body.begin(); it != graph.body.end(); ++it)
    {
        it->position.x *= m_dimensions.x;
        it->position.y *= m_dimensions.y;
        m_points.emplace_back(it->type, it->position);
    }
    for (int i = 0; i < graph.body.size(); ++i)
    {
        for (int j = 0; j < graph.body[i].neighbours.size(); ++j)
        {
            int k = graph.body[i].neighbours[j];
            m_points[i].addNeighbour(k, m_points[k].getPosition());
        }
    }
}

void World::drawEverything(sf::RenderWindow& window)
{
    for (auto it = m_points.begin(); it != m_points.end(); ++it)
        it->drawLines(window);
    for (auto it = m_points.begin(); it != m_points.end(); ++it)
        it->drawCircle(window);
}

int World::getRandomSource()
{
    if (m_sources_number)
        return rand() % m_sources_number;
    throw Error(Problem::OutOfRange);
}

int World::getRandomNeighbour(int index)
{
    try
    {
        return m_points.at(index).randomNeighbour();
    }
    catch (...)
    {
        throw Error(Problem::OutOfRange);
    }
}

sf::Vector2f World::getCoords(int index)
{
    try
    {
        return m_points.at(index).getPosition();
    }
    catch (...)
    {
        throw Error(Problem::OutOfRange);
    }
}

PointType World::getType(int index)
{
    try
    {
        return m_points.at(index).getType();
    }
    catch (...)
    {
        throw Error(Problem::OutOfRange);
    }
}