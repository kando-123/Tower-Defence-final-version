#pragma once
#include "button.h"
#include "defence.h"
#include "entity.h"
#include "manager.h"
#include "shop.h"
#include "world.h"
#include <list>
#include <memory>
#include <queue>
#include <vector>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

const unsigned int WINDOW_WIDTH = 1200U, WINDOW_HEIGHT = 740, FREQUENCY = 60;
const float WORLD_WIDTH = 1000.f, WORLD_HEIGHT = 740.f;
const int INITIAL_HEALTH = 200, INITIAL_MONEY = 120, SPAWN_PERIOD = 30, ATTACK_PERIOD = 15;;
const float OUTLINE_THICKNESS = 2.f;
const sf::Color BUTTON_FILL(0x00, 0xc0, 0xf0), BUTTON_OUTLINE(0x00, 0x60, 0x90),
	HEALTH_COLOR(0xf0, 0x00, 0x00), MONEY_COLOR(0xff, 0x80, 0x00),
	RANGE_COLOR(0x80, 0x80, 0xff, 0x80);

enum class Result { Interrupt, Victory, Failure };

class Engine
{
private:

	std::unique_ptr<sf::RenderWindow> m_window_ptr;

	World& m_world_ref;
	Manager& m_manager_ref;

	Button m_start_button;

	// level //

	Level m_level;
	bool m_spawning = false;
	bool m_fighting = false;

	// entities //

	std::list<Entity> m_entities;
	sf::Text m_health_bar;
	int m_health = 0;

	// defences //

	sf::Text m_money_bar;
	int m_money = 0;
	Shop& m_shop_ref;
	std::vector<std::unique_ptr<Defence>> m_defences;
	std::unique_ptr<Defence> m_holder;
	std::unique_ptr<sf::CircleShape> m_defence_range;
	std::vector<std::list<Entity>::iterator> m_dividers;
	int m_inserter = 1;
	int m_attack_counter = ATTACK_PERIOD;

	// end of game //

	bool m_game_over;
	Result m_result;

	// -- Methods -- //

	void serveEvents();
	void serveLeftButton();
	void spawnEntity();
	void doAttacking();

	Engine();

public:
	static Engine& getInstance()
	{
		static Engine instance;
		return instance;
	}

	void prepare();
	bool running();
	void update();
	void render();
	void finish();

	Result getResult();
};
