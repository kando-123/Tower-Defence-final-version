#include "engine.h"
#include "error.h"
#include <mutex>
#include <numbers>

#include <iostream>

void Engine::serveEvents()
{
	static sf::Event s_event;
	while (m_window_ptr->pollEvent(s_event))
	{
		switch (s_event.type)
		{
		case sf::Event::Closed:
			m_window_ptr->close();
			break;
		case sf::Event::KeyPressed:
			switch (s_event.key.code)
			{
			case sf::Keyboard::Escape:
				m_window_ptr->close();
				break;
			}
			break;
		case sf::Event::MouseButtonPressed:
			if (s_event.mouseButton.button == sf::Mouse::Left)
				serveLeftButton();
			break;
		}
	}
}

void Engine::serveLeftButton()
{
	sf::Vector2f mouse_position =
		static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window_ptr));
	if (not m_fighting and m_start_button.contains(mouse_position))
	{
		m_spawning = true;
		m_fighting = true;
		m_start_button.toggle();
	}
	if (m_holder == nullptr and mouse_position.x > m_world_ref.getDimensions().x)
	{
		DefenceType defence_type = m_shop_ref.select(mouse_position);
		if (defence_type != DefenceType::None)
		{
			const DefenceRecord& record = m_manager_ref.getDefenceRecord(defence_type);
			if (m_money >= record.cost)
			{
				m_shop_ref.assignDefence(defence_type, m_holder);
				m_shop_ref.toggleButton();
				float radius = m_holder->getRadius();
				m_defence_range = std::make_unique<sf::CircleShape>();
				m_defence_range->setFillColor(RANGE_COLOR);
				m_defence_range->setRadius(radius);
				m_defence_range->setPosition(mouse_position.x - radius, mouse_position.y - radius);
			}
		}
	}
	else if (m_holder != nullptr and mouse_position.x < m_world_ref.getDimensions().x)
	{
		m_money -= m_holder->getCost();
		m_money_bar.setString("Money = " + std::to_string(m_money));
		m_holder->setPosition(mouse_position);
		m_defences.emplace_back(m_holder.release());
		m_shop_ref.toggleButton();
		m_dividers.emplace_back(m_entities.end());
		m_defence_range.reset();
	}
}

void Engine::spawnEntity()
{
	if (m_level.empty())
		return;
	int entityIndex = m_level.front().front().index;
	if (--m_level.front().front().count == 0)
	{
		m_level.front().pop();
		if (m_level.front().empty())
		{
			m_level.pop();
			m_spawning = false;
		}
	}
	if (m_defences.empty())
	{
		m_entities.emplace_back(entityIndex);
		m_dividers.front() = m_entities.begin();
	}
	else
	{
		m_entities.emplace(m_dividers[m_inserter], entityIndex);		
		for (int i = m_inserter - 1; i >= 0; --i)
		{
			if (m_dividers[i] == m_dividers[m_inserter])
				--m_dividers[i];
		}
		if (++m_inserter == m_dividers.size())
			m_inserter = 1;
	}
}

void Engine::doAttacking()
{
	for (int i = 0; i < m_defences.size(); ++i)
		m_defences[i]->tick();

	std::vector<std::thread> threads;

	auto make_attack = [] (Defence& def,
		std::list<Entity>::iterator first,
		std::list<Entity>::iterator last)
	{
		def.attack(first, last);
	};

	for (int i = 0; i < m_defences.size(); ++i)
	{
		bool anyone_ready = false;
		for (int j = 0; j < m_defences.size(); ++j)
		{
			if (m_defences[j]->ready())
			{
				anyone_ready = true;
				int k = (j + i) % m_defences.size();
				threads.emplace_back(make_attack,
					std::ref(*m_defences[j]),
					m_dividers[k],
					m_dividers[k + 1]);
			}
		}
		for (int j = 0; j < threads.size(); ++j)
			threads[j].join();
		threads.clear();
		if (not anyone_ready)
			break;
	}

	auto reset_all = [](std::vector<std::unique_ptr<Defence>>::iterator first,
		std::vector<std::unique_ptr<Defence>>::iterator last)
	{
		while (first != last)
		{
			(*first)->reset();
			++first;
		}
	};
	
	std::jthread jthread(reset_all, m_defences.begin(), m_defences.end());

	auto it = m_entities.begin();
	while (it != m_entities.end())
	{
		if (it->isAlive())
			++it;
		else
		{
			for (int i = 0; i < m_dividers.size(); ++i)
			{
				if (m_dividers[i] == it)
					++m_dividers[i];
			}
			m_money += m_manager_ref.getEntityRecord(it->getType()).prize;
			it = m_entities.erase(it);
		}
	}
	m_money_bar.setString("Money = " + std::to_string(m_money));
}

Engine::Engine() :
	m_world_ref(World::getInstance()),
	m_manager_ref(Manager::getInstance()),
	m_shop_ref(Shop::getInstance())
{
	std::srand(static_cast<unsigned int>(std::time(0)));

	m_window_ptr = std::make_unique<sf::RenderWindow>(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
		"Tower Defence: Setup", sf::Style::Close);
	m_window_ptr->setFramerateLimit(FREQUENCY);
	m_world_ref.setDimensions(WORLD_WIDTH, WORLD_HEIGHT);

	m_spawning = false;
	m_fighting = false;

	const sf::Font& font = m_manager_ref.shareFont();

	m_health = INITIAL_HEALTH;
	m_health_bar.setFont(font);

	m_health_bar.setString("Health = " + std::to_string(m_health));
	m_health_bar.setFillColor(HEALTH_COLOR);
	m_health_bar.setPosition(WORLD_WIDTH, 0.f);

	m_money = INITIAL_MONEY;

	m_money_bar.setFont(font);
	m_money_bar.setString("Money = " + std::to_string(m_money));
	m_money_bar.setFillColor(MONEY_COLOR);
	m_money_bar.setPosition(WORLD_WIDTH, TEXT_SIZE);

	float button_width = static_cast<float>(WINDOW_WIDTH) - WORLD_WIDTH;
	float button_height = button_width / std::numbers::phi_v<float>;

	m_start_button.setSize(button_width, button_height);
	m_start_button.setColors(BUTTON_FILL, BUTTON_OUTLINE);
	m_start_button.setPosition(WORLD_WIDTH, WORLD_HEIGHT - button_height);
	m_start_button.setOutlineThickness(OUTLINE_THICKNESS);
	m_start_button.m_text.setString("Start");
	m_start_button.m_text.setFont(m_manager_ref.shareFont());

	m_shop_ref.setPosition(WORLD_WIDTH, 2 * TEXT_SIZE);
	m_shop_ref.setSize(button_width, WORLD_HEIGHT - button_height - 2 * TEXT_SIZE);

	m_dividers.push_back(m_entities.begin());
	m_holder = nullptr;

	m_result = Result::Interrupt;
	m_game_over = false;
}

void Engine::prepare()
{
	try
	{
		m_manager_ref.loadFont();

		std::string map_name, level_name;

		m_manager_ref.checkMaps();
		m_manager_ref.loadMap(*m_window_ptr, map_name);

		m_manager_ref.readEntitiesData();

		m_manager_ref.checkLevels();
		m_manager_ref.loadLevel(*m_window_ptr, m_level, level_name);

		m_window_ptr->setTitle("Gameplay: " + map_name + ", " + level_name);

		m_manager_ref.readDefencesData();

		m_shop_ref.makeButtons(m_manager_ref.shareFont());
	}
	catch (Error err)
	{
		if (err.problem() == Problem::Interrupt)
			m_game_over = true;
		else
			throw;
	}
}

bool Engine::running()
{
	if (not m_game_over and m_window_ptr != nullptr)
		return m_window_ptr->isOpen();
	return false;
}

void Engine::update()
{
	if (m_defence_range != nullptr)
	{
		float radius = m_defence_range->getRadius();
		sf::Vector2f mouse_position =
			static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window_ptr));
		m_defence_range->setPosition(mouse_position.x - radius, mouse_position.y - radius);
	}
	serveEvents();
	static int spawn_counter = 1;
	if (m_spawning)
	{
		if (--spawn_counter == 0)
		{
			spawnEntity();
			spawn_counter = SPAWN_PERIOD;
		}
	}
	if (--m_attack_counter <= 0)
	{
		m_attack_counter = ATTACK_PERIOD;
		if (not m_defences.empty() and not m_entities.empty())
			doAttacking();
	}

	auto it = m_entities.begin();
	while (it != m_entities.end())
	{
		if (it->move())
			++it;
		else
		{
			m_health -= m_manager_ref.getEntityRecord(it->getType()).force;
			if (m_health <= 0)
			{
				m_result = Result::Failure;
				m_game_over = true;
			}
			m_health_bar.setString("Health = " + std::to_string(m_health));
			for (int i = 0; i < m_dividers.size(); ++i)
			{
				if (m_dividers[i] == it)
					++m_dividers[i];
			}
			it = m_entities.erase(it);
		}
	}
	if (m_fighting and not m_spawning and m_entities.empty())
	{
		m_fighting = false;
		spawn_counter = 1;
		m_start_button.toggle();
		if (m_level.empty())
		{
			m_game_over = true;
			m_result = Result::Victory;
		}
		for (int i = 0; i < m_dividers.size(); ++i)
			m_dividers[i] = m_entities.begin();
		m_inserter = 1;
	}
}

void Engine::render()
{
	m_window_ptr->clear();
	m_world_ref.drawEverything(*m_window_ptr);
	for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
		it->drawYourself(*m_window_ptr);
	for (auto it = m_defences.begin(); it != m_defences.end(); ++it)
		(*it)->drawYourself(*m_window_ptr);
	m_shop_ref.drawYourself(*m_window_ptr);
	m_window_ptr->draw(m_health_bar);
	m_window_ptr->draw(m_money_bar);
	m_start_button.drawYourself(*m_window_ptr);
	if (m_defence_range != nullptr)
		m_window_ptr->draw(*m_defence_range);
	m_window_ptr->display();
}

void Engine::finish()
{
	sf::Text result, comment;
	result.setPosition(.1f * WINDOW_WIDTH, .1f * WINDOW_HEIGHT);
	comment.setPosition(.1f * WINDOW_WIDTH, .1f * WINDOW_HEIGHT + 2.f * TEXT_SIZE);
	result.setCharacterSize(TEXT_SIZE);
	comment.setCharacterSize(.5f * TEXT_SIZE);
	result.setFont(m_manager_ref.shareFont());
	comment.setFont(m_manager_ref.shareFont());
	switch (m_result)
	{
	case Result::Victory:
		result.setString("Victory!");
		result.setFillColor(sf::Color::Green);
		break;
	case Result::Failure:
		result.setString("Failure...");
		result.setFillColor(sf::Color::Red);
		break;
	default:
		result.setString("Game interrupted...");
		result.setFillColor(sf::Color::Blue);
	}
	comment.setString("Close the window to finish.");
	result.setStyle(sf::Text::Style::Bold);
	comment.setStyle(sf::Text::Style::Italic);
	sf::Event event;
	while (m_window_ptr->isOpen())
	{
		m_window_ptr->clear();
		while (m_window_ptr->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				m_window_ptr->close();
		}
		m_window_ptr->draw(result);
		m_window_ptr->draw(comment);
		m_window_ptr->display();
	}
	m_window_ptr.reset();
}

Result Engine::getResult()
{
	return m_result;
}
