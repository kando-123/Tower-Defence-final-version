#include "shop.h"
#include "manager.h"
#include <ranges>

Shop::Shop()
{
	m_background.setFillColor(SHOP_BACKGROUND);
	m_currently_selected = DefenceType::None;
}

void Shop::makeButtons(const sf::Font& font)
{
	Manager& manager_ref = Manager::getInstance();
	sf::Vector2f coords = m_background.getPosition();
	float width = m_background.getSize().x;
	for (auto it = NAMES.begin(); it != NAMES.end(); ++it)
	{
		std::pair<DefenceType, Button> pair;
		pair.first = it->first;
		int cost = manager_ref.getDefenceRecord(it->first).cost;
		pair.second.m_text.setString(it->second + ", " + std::to_string(cost));
		pair.second.m_text.setFont(font);
		pair.second.setColors(SHOP_BUTTON_FILL, SHOP_BUTTON_LINE);
		pair.second.setSize(.9f * width, .9f * TEXT_SIZE);
		pair.second.setPosition(coords.x + .05f * width, coords.y + .05f * TEXT_SIZE);
		coords.y += TEXT_SIZE;
		pair.second.setOutlineThickness(SHOP_BUTTON_LINE_THICKNESS);
		m_buttons.insert(pair);
	}
}

void Shop::setPosition(float x, float y)
{
	m_background.setPosition(x, y);
	std::ranges::for_each(
		m_buttons | std::views::values,
		[x, y](Button& b) -> void { b.move(x, y); } );
}

void Shop::setSize(float width, float height)
{
	m_background.setSize(sf::Vector2f(width, height));
}

void Shop::drawYourself(sf::RenderWindow& window)
{
	window.draw(m_background);
	for (auto it = m_buttons.begin(); it != m_buttons.end(); ++it)
		it->second.drawYourself(window);
}

DefenceType Shop::select(const sf::Vector2f& coords)
{
	for (auto it = m_buttons.begin(); it != m_buttons.end(); ++it)
	{
		if (it->second.contains(coords))
		{
			m_currently_selected = it->first;
			return m_currently_selected;
		}
	}
	return DefenceType::None;
}

void Shop::toggleButton()
{
	m_buttons[m_currently_selected].toggle();
}

void Shop::assignDefence(DefenceType type, std::unique_ptr<Defence>& pointer)
{
	Manager& manager_ref = Manager::getInstance();
	const DefenceRecord& record = manager_ref.getDefenceRecord(type);
	switch (type)
	{
	case DefenceType::UniShooter:
	case DefenceType::MultiShooter:
	case DefenceType::Cannon:
		pointer = std::make_unique<Shooter>();
		break;
	case DefenceType::Freezer:
		pointer = std::make_unique<Freezer>();
		break;
	default:
		pointer = nullptr;
	}
	if (pointer != nullptr)
	{
		pointer->setForce(record.force);
		pointer->setHitsPerOnce(record.hits);
		pointer->setPeriod(record.period);
		pointer->setRadius(record.radius);
		pointer->setCost(record.cost);
		pointer->setTexture(record.texture);
		sf::Vector2f offset = -.5f * record.scale * record.dimensions;
		pointer->setScale(record.scale);
		pointer->setShift(offset);
	}
}
