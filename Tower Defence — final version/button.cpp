#include "button.h"

void Button::setSize(float width, float height)
{
	m_background.setSize(sf::Vector2f(width, height));
	m_text.setCharacterSize(.4f * height);
}

void Button::setPosition(float x, float y)
{
	m_background.setPosition(x, y);
	sf::Vector2f text_position;
	text_position.x = x + .1f * m_background.getSize().x;
	text_position.y = y + .25f * m_background.getSize().y;
	m_text.setPosition(text_position);
}

void Button::move(float x_offset, float y_offset)
{
	m_background.move(x_offset, y_offset);
	m_text.move(x_offset, y_offset);
}

void Button::setColors(const sf::Color& color_1, const sf::Color& color_2)
{
	m_background.setFillColor(color_1);
	m_background.setOutlineColor(color_2);
	m_text.setFillColor(color_2);
}

void Button::setOutlineThickness(float thickness)
{
	m_background.setOutlineThickness(thickness);
	m_background.move(-thickness, -thickness);
	m_text.move(-thickness, -thickness);
}

void Button::drawYourself(sf::RenderWindow& window)
{
	window.draw(m_background);
	window.draw(m_text);
}

bool Button::contains(const sf::Vector2f& coords)
{
	// ?
	return m_background.getGlobalBounds().contains(coords);
}

void Button::toggle()
{
	sf::Color color = m_background.getFillColor();
	m_background.setFillColor(m_background.getOutlineColor());
	m_background.setOutlineColor(color);
	m_text.setFillColor(color);
}
