#pragma once
#include "graph.h"
#include "defence.h"
#include <filesystem>
#include <map>
#include <queue>
#include <vector>
#include <unordered_map>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

struct EntityRecord
{
	float speed = 0.f;
	int health = 1, force = 0, prize = 0;
	sf::Vector2f dimensions;
	float scale = 1.f;
	std::filesystem::path texture_path = "";
	sf::Texture texture;

	EntityRecord() = default;
	bool loadTexture();
};

struct Group
{
	int index; // index of the entity
	int count;
	Group(int new_index, int new_count);
};

using Wave = std::queue<Group>;
using Level = std::queue<Wave>;

struct DefenceRecord
{
	float radius = 0.f;
	int period = 1;
	int force = 0;
	int hits = 0;
	int cost = 0;
	sf::Vector2f dimensions;
	float scale;
	std::filesystem::path texture_path = "";
	sf::Texture texture;

	DefenceRecord() = default;
	bool loadTexture();
};

const int TOP = 0, BOTTOM = 1, LEFT = 2, RIGHT = 3;
const float MARGIN[4]{ 70.f, 50.f, 40.f, 40.f };
const float TEXT_SIZE = 60.f, MINIMAL_GAP = 50.f;

const std::string FONT_FILE = "arial.ttf";
const std::string MAPS_DIR = "Maps", MAP_EXTENSION = ".tdm";
const std::string LEVELS_DIR = "Levels", LEVEL_EXTENSION = ".tdl";
const std::string ENTITIES_DIR = "Entities", ENTITIES_SOURCE = "Entities.tde";
const std::string DEFENCES_DIR = "Defences", DEFENCES_SOURCE = "Defences.tdd";

bool extractFile(const std::filesystem::path& source, std::string& map_name, Graph& graph);
bool findSimpleErrors(Graph& graph);
bool checkConnectedness(Graph& graph);
void refactorGraph(Graph& graph);

class Manager
{
private:

	// texts //

	std::vector<sf::Text> m_texts;
	sf::Font m_arial;
	int m_skipped_count = 0;
	int m_shown_count = 0;
	bool m_overflow = false;

	bool withinMargins(const sf::Vector2f& coords, const sf::Vector2f& window_dimensions);
	int overflowSelection(sf::RenderWindow& window, sf::Text& request);
	int normalSelection(sf::RenderWindow& window, sf::Text& request);
	std::string selectText(sf::RenderWindow& window, sf::String request_text);

	// maps //

	std::map<std::string, std::filesystem::path> m_maps_dictionary; // name-to-files

	void prepareMapNames(float window_height);

	// entities //

	std::unordered_map<std::string, int> m_entities_dictionary; // name-to-index
	std::vector<EntityRecord> m_entities_data;

	// levels //

	std::unordered_map<std::string, std::filesystem::path> m_levels_dictionary; // names-to-files
	
	bool validLevel(const std::filesystem::path& source, std::string& file_name);
	bool readLevel(const std::filesystem::path& source, Level& level);
	void prepareLevelNames(float window_height);

	// buildings //

	std::unordered_map<DefenceType, DefenceRecord> m_defences_data;

	// constructor //

	Manager() = default;

public:

	static Manager& getInstance()
	{
		static Manager instance;
		return instance;
	}

	void loadFont();
	const sf::Font& shareFont();

	void checkMaps();
	void loadMap(sf::RenderWindow& window, std::string& map_name);

	void checkLevels();
	void loadLevel(sf::RenderWindow& window, Level& level, std::string& map_name);

	void readEntitiesData();
	const EntityRecord& getEntityRecord(int index);

	void readDefencesData();
	DefenceRecord& getDefenceRecord(DefenceType type);
};
