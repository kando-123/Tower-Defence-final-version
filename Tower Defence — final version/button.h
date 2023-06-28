#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

struct Button
{
	sf::RectangleShape m_background;
	sf::Text m_text;

	Button() = default;

	void setSize(float width, float height);
	void setPosition(float x, float y);
	void move(float x_offset, float y_offset);
	void setColors(const sf::Color& color_1, const sf::Color& color_2);
	void setOutlineThickness(float thickness);

	void drawYourself(sf::RenderWindow& window);
	bool contains(const sf::Vector2f& coords);

	void toggle();
};
