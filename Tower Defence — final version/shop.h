#pragma once
#include "button.h"
#include "defence.h"
#include <map>
#include <unordered_map>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>


const std::map<DefenceType, std::string>
	NAMES { { DefenceType::UniShooter, "Unishooter" },
			{ DefenceType::MultiShooter, "Multishooter" },
			{ DefenceType::Cannon, "Cannon" },
			{ DefenceType::Freezer, "Freezer" } };
const float SHOP_BUTTON_HEIGHT = 40.f;
const float SHOP_BUTTON_LINE_THICKNESS = 2.f;
const sf::Color SHOP_BUTTON_FILL(0xf0, 0xf0, 0xf0);
const sf::Color SHOP_BUTTON_LINE(0x0f, 0x0f, 0x0f);
const sf::Color SHOP_BACKGROUND(0x80, 0x80, 0x80);

class Shop
{
private:

	sf::RectangleShape m_background;
	std::map<DefenceType, Button> m_buttons;
	DefenceType m_currently_selected;

	Shop();

public:

	static Shop& getInstance()
	{
		static Shop instance;
		return instance;
	}

	void makeButtons(const sf::Font& font);

	void setPosition(float x, float y);
	void setSize(float width, float height);

	void drawYourself(sf::RenderWindow& window);

	DefenceType select(const sf::Vector2f& coords);
	void toggleButton();

	void assignDefence(DefenceType type, std::unique_ptr<Defence>& pointer);
};
