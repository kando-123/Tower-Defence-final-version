#include "manager.h"
#include "engine.h"
#include "error.h"
#include "world.h"
#include <cmath>
#include <fstream>
#include <future>
#include <ranges>
#include <regex>
#include <string>
#include <unordered_map>
#include <thread>

bool EntityRecord::loadTexture()
{
    if (std::filesystem::exists(texture_path))
        return texture.loadFromFile(texture_path.string());
    return false;
}

Group::Group(int new_index, int new_count)
{
    index = new_index;
    count = new_count;
}

bool DefenceRecord::loadTexture()
{
    if (std::filesystem::exists(texture_path))
        return texture.loadFromFile(texture_path.string());
    return false;
}

bool Manager::withinMargins(const sf::Vector2f& coords, const sf::Vector2f& window_dimensions)
{
    return
        coords.x >= MARGIN[LEFT] and
        coords.y >= MARGIN[TOP] and
        coords.x <= window_dimensions.x - MARGIN[RIGHT] and
        coords.y <= window_dimensions.y - MARGIN[BOTTOM];

}

int Manager::overflowSelection(sf::RenderWindow& window, sf::Text& request)
{
    m_skipped_count = 0;
    sf::Event event = sf::Event();
    bool ready = false, redraw = true;
    int selected = -1;
    do {
        if (redraw)
        {
            window.clear();
            window.draw(request);
            float shift = m_skipped_count * TEXT_SIZE;
            auto draw = [&window, shift](sf::Text text) -> void
            {
                sf::Vector2f coords = text.getPosition();
                coords.y -= shift;
                text.setPosition(coords);
                window.draw(text);
            };
            std::ranges::for_each(m_texts
                | std::views::drop(m_skipped_count)
                | std::views::take(m_shown_count),
                draw);
            window.display();
            redraw = false;
        }
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2f mouse_position = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                    sf::Vector2f window_size = static_cast<sf::Vector2f>(window.getSize());
                    if (withinMargins(mouse_position, window_size))
                    {
                        selected = (mouse_position.y - MARGIN[TOP]) / TEXT_SIZE + m_skipped_count;
                        ready = true;
                    }
                }
                break;
            case sf::Event::MouseWheelScrolled:
            {
                int scroll = static_cast<int>(event.mouseWheelScroll.delta);
                int upper_limit = static_cast<int>(m_texts.size()) - m_shown_count;
                m_skipped_count = std::max(std::min(m_skipped_count - scroll, upper_limit), 0);
            }
                redraw = true;
                break;
            case sf::Event::Closed:
                window.close();
                throw Error(Problem::Interrupt);
            }
        }
    } while (not ready);
    return selected;
}

int Manager::normalSelection(sf::RenderWindow& window, sf::Text& request)
{
    sf::Event event = sf::Event();
    bool ready = false;
    int selected = -1;
    window.draw(request);
    for (const auto& n : m_texts)
        window.draw(n);
    window.display();
    do {
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2f mouse_position = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                    sf::Vector2f window_size = static_cast<sf::Vector2f>(window.getSize());
                    if (withinMargins(mouse_position, window_size))
                    {
                        selected = (mouse_position.y - MARGIN[TOP]) / TEXT_SIZE;
                        if (selected < m_shown_count)
                            ready = true;
                    }
                }
                break;
            case sf::Event::Closed:
                window.close();
                throw Error(Problem::Interrupt);
            }
        }
    } while (not ready);
    return selected;
}

std::string Manager::selectText(sf::RenderWindow& window, sf::String request_text)
{
    sf::Text request(request_text, m_arial);
    request.setPosition(MARGIN[LEFT], MARGIN[TOP] - TEXT_SIZE);
    int selected = -1;
    if (m_overflow)
        selected = overflowSelection(window, request);
    else
        selected = normalSelection(window, request);
    return m_texts[selected].getString();
}

bool extractFile(const std::filesystem::path& source, std::string& map_name, Graph& graph)
{
    graph.body.clear();
    graph.sources_count = 0;
    graph.towers_count = 0;
    if (not std::filesystem::exists(source))
        throw Error(Problem::FileError);
    std::ifstream file(source);
    if (file.bad())
        throw Error(Problem::FileError);
    const std::regex
        name_line(R"((?:\s*(?:#.*\n\s*)?)*)"
            R"(name: ([a-zA-Z0-9 +\-#&%\(\)]{3,}))"), // CG \1 -> name of the map
        point_definition(R"((?:\s*(?:#.*\n\s*)?)*)"
            R"((source|vertex|tower)[ \t]+(\w+)[ \t]+)" // CG \1 -> type & CG \2 -> marking
            R"(x ?= ?((?:\.\d+)|(?:0(?:\.\d*)?)|(?:1(?:\.0*)?))[ \t]+)" // CG \3 -> abscissa
            R"(y ?= ?((?:\.\d+)|(?:0(?:\.\d*)?)|(?:1(?:\.0*)?))[ \t]*)"), // CG \4 -> ordinate
        connection(R"((?:\s*(?:#.*\n\s*)?)*)"
            R"((\w+)[ \t]+(\w+)[ \t]*)"); // CG \1 -> tail_index & CG \2 -> head_index
    std::string piece;
    std::smatch result;
    sf::Vector2f coords;
    if (not std::getline(file, piece, ';') or not std::regex_match(piece, result, name_line))
    {
        file.close();
        return false;
    }
    map_name = std::string(result[1].first, result[1].second);
    std::unordered_map<std::string, int> markings;
    int count = 0;
    bool failure = false;
    do {
        if (not std::getline(file, piece, ';'))
        {
            failure = true;
            break;
        }
        if (std::regex_match(piece, result, point_definition))
        {
            std::string type_string(result[1].first, result[1].second);
            PointType type = PointType::Vertex;
            if (type_string == "source")
            {
                type = PointType::Source;
                ++graph.sources_count;
            }
            else if (type_string == "tower")
            {
                type = PointType::Tower;
                ++graph.towers_count;
            }
            std::string marking(result[2].first, result[2].second);
            if (markings.find(marking) != markings.end())
            {
                failure = true; // ambiguous marking
                break;
            }
            markings[marking] = count++;
            coords.x = std::stof(std::string(result[3].first, result[3].second));
            coords.y = std::stof(std::string(result[4].first, result[4].second));
            graph.body.emplace_back(Element(type, coords));
        }
        else
            break;
    } while (not failure);
    do {
        if (failure)
            break;
        if (not std::regex_match(piece, result, connection))
        {
            failure = true;
            break;
        }
        std::string tail_marking = std::string(result[1].first, result[1].second),
            head_marking = std::string(result[2].first, result[2].second);
        if (markings.find(tail_marking) == markings.end() or
            markings.find(head_marking) == markings.end())
        {
            failure = true; // a connection defined between points absent in the graph
            break;
        }
        if (tail_marking == head_marking)
        {
            failure = true; // no loops allowed
            break;
        }
        int tail_index = markings[tail_marking],
            head_index = markings[head_marking];
        graph.body[tail_index].neighbours.push_back(head_index);
        graph.body[head_index].antineighbours.push_back(tail_index);
    } while (not failure and std::getline(file, piece, ';'));
    file.close();
    return not failure;
}

bool findSimpleErrors(Graph& graph)
{
    sf::Vector2f world_dimensions = World::getInstance().getDimensions();
    if (graph.sources_count == 0 or graph.towers_count == 0)
        return false;
    for (int i = 0; i < graph.body.size(); ++i)
    {
        for (int j = 0; j < graph.body[i].neighbours.size(); ++j)
        {
            int k = graph.body[i].neighbours[j];
            float difference_x = (graph.body[i].position.x - graph.body[k].position.x) * world_dimensions.x;
            float difference_y = (graph.body[i].position.y - graph.body[k].position.y) * world_dimensions.y;
            if (std::fabs(difference_x) < MINIMAL_GAP and std::fabs(difference_y) < MINIMAL_GAP)
                return false; // too close to each other
        }
    }
    for (int i = 0; i < graph.body.size(); ++i)
    {
        if (graph.body[i].type != PointType::Tower)
        {
            if (graph.body[i].neighbours.empty())
                return false; // nowhere to go to
        }
        else if (graph.body[i].antineighbours.empty() or not graph.body[i].neighbours.empty())
            return false; // nowhere to come from
    }
    return true;
}

bool checkConnectedness(Graph& graph)
{
    for (int i = 0; i < graph.body.size(); ++i)
        graph.body[i].visited = false;
    int sources_count = graph.sources_count;
    std::vector<int> stack;
    for (int i = 0; i < graph.body.size(); ++i)
    {
        if (graph.body[i].visited)
            continue;
        if (graph.body[i].type == PointType::Tower)
        {
            int j = i;
            bool forward = true;
            do {
                if (forward)
                {
                    graph.body[j].visited = true;
                    if (graph.body[j].type == PointType::Source)
                    {
                        --sources_count;
                        if (sources_count == 0)
                            return true; // SUCCESS!!! all the sources are connected
                    }
                    stack.push_back(j);
                }
                forward = false;
                for (int k = 0; k < graph.body[j].antineighbours.size(); ++k)
                {
                    if (not graph.body[graph.body[j].antineighbours[k]].visited)
                    {
                        forward = true;
                        j = graph.body[j].antineighbours[k];
                        break;
                    }
                }
                if (not forward)
                {
                    stack.pop_back();
                    if (not stack.empty())
                        j = stack.back();
                }
            } while (not stack.empty());
        }
    }
    return false;
}

void refactorGraph(Graph& graph)
{
    for (int i = 0; i < graph.sources_count; ++i)
    {
        if (graph.body[i].type == PointType::Source)
            continue;
        for (int j = i + i; j < graph.body.size(); ++j)
        {
            if (graph.body[j].type == PointType::Source)
            {
                int new_index = i, old_index = j;
                std::swap(graph.body[i], graph.body[j]);
                for (j = 0; j < graph.body.size(); ++j)
                {
                    for (int k = 0; k < graph.body[j].neighbours.size(); ++k)
                    {
                        if (graph.body[j].neighbours[k] == new_index)
                            graph.body[j].neighbours[k] = old_index;
                        else if (graph.body[j].neighbours[k] == old_index)
                            graph.body[j].neighbours[k] = new_index;
                    }
                    for (int k = 0; k < graph.body[j].antineighbours.size(); ++k)
                    {
                        if (graph.body[j].antineighbours[k] == new_index)
                            graph.body[j].antineighbours[k] = old_index;
                        else if (graph.body[j].antineighbours[k] == old_index)
                            graph.body[j].antineighbours[k] = new_index;
                    }
                }
                break;
            }
        }
    }
}

void Manager::prepareMapNames(float window_height)
{
    m_texts.clear();
    float y_coord = MARGIN[TOP];
    for (const std::string& name : m_maps_dictionary | std::ranges::views::keys)
    {
        m_texts.emplace_back(name, m_arial);
        m_texts.back().setPosition(MARGIN[LEFT], y_coord);
        y_coord += TEXT_SIZE;
    }
    if (MARGIN[TOP] + m_texts.size() * TEXT_SIZE + MARGIN[BOTTOM] > window_height)
    {
        m_overflow = true;
        m_shown_count = static_cast<int>((window_height - MARGIN[TOP] - MARGIN[BOTTOM]) / TEXT_SIZE);
    }
    else
        m_shown_count = m_texts.size();
}

bool Manager::validLevel(const std::filesystem::path& source, std::string& file_name)
{
    if (not std::filesystem::exists(source))
        throw Error(Problem::FileError);
    std::ifstream file(source);
    if (file.bad())
        throw Error(Problem::FileError);
    const std::regex
        name_line(R"((?:\s*(?:#.*\n\s*)?)*level:[ \t]+(.{3,}))"), // CG \1 -> name of the level
        next_keyword(R"((?:\s*(?:#.*\n\s*)?)*next)"),
        end_keyword(R"((?:\s*(?:#.*\n\s*)?)*end)"),
        group_definition(R"((?:\s*(?:#.*\n\s*)?)*(\w+)[ \t]+(\d+))");
    std::string piece;
    std::smatch result;
    int damage = 0;
    if (not std::getline(file, piece, ';') or not std::regex_match(piece, result, name_line))
    {
        file.close();
        return false;
    }
    file_name = std::string(result[1].first, result[1].second);
    if (not std::getline(file, piece, ';'))
    {
        file.close();
        return false;
    }
    bool keyword = true;
    bool success = false, failure = false;
    do {
        if (std::regex_match(piece, result, group_definition))
        {
            keyword = false;
            std::string label(result[1].first, result[1].second);
            int count = std::stoi(std::string(result[2].first, result[2].second));
            if (m_entities_dictionary.find(label) == m_entities_dictionary.end() or count == 0)
                failure = true;
            else
            {
                int index = m_entities_dictionary[label];
                int force = m_entities_data[index].force;
                damage += count * force;
            }
        }
        else
        {
            if (std::regex_match(piece, next_keyword))
            {
                if (keyword)
                    failure = true;
                else
                    keyword = true;
            }
            else
            {
                if (std::regex_match(piece, end_keyword))
                {
                    if (keyword)
                        failure = true;
                    else
                        success = true;
                }
                else
                    failure = true;

            }
        }
    } while (not success and not failure and std::getline(file, piece, ';'));
    file.close();
    if (damage < INITIAL_HEALTH)
        success = false;
    return success;
}

bool Manager::readLevel(const std::filesystem::path& source, Level& level)
{
    if (not std::filesystem::exists(source))
        throw Error(Problem::FileError);
    std::ifstream file(source);
    if (file.bad())
        throw Error(Problem::FileError);
    const std::regex
        name_line(R"((?:\s*(?:#.*\n\s*)?)*level:[ \t]+(.{3,}))"),
        next_statement(R"((?:\s*(?:#.*\n\s*)?)*next)"),
        end_statement(R"((?:\s*(?:#.*\n\s*)?)*end)"),
        group_definition(R"((?:\s*(?:#.*\n\s*)?)*(\w+)[ \t]+(\d+))");
    std::string piece;
    std::smatch result;
    int damage = 0;
    if (not std::getline(file, piece, ';') or not std::regex_match(piece, result, name_line))
    {
        file.close();
        return false;
    }
    if (not std::getline(file, piece, ';'))
    {
        file.close();
        return false;
    }
    bool keyword = true;
    bool success = false, failure = false;
    level.emplace();
    do {
        if (std::regex_match(piece, result, group_definition))
        {
            keyword = false;
            std::string label(result[1].first, result[1].second);
            int count = std::stoi(std::string(result[2].first, result[2].second));
            if (m_entities_dictionary.find(label) == m_entities_dictionary.end() or count == 0)
                failure = true;
            else
            {
                int index = m_entities_dictionary[label];
                int force = m_entities_data[index].force;
                damage += count * force;
                level.back().emplace(index, count);
            }
        }
        else
        {
            if (std::regex_match(piece, next_statement))
            {
                level.emplace();
                if (keyword)
                    failure = true;
                else
                    keyword = true;
            }
            else
            {
                if (std::regex_match(piece, end_statement))
                {
                    if (keyword)
                        failure = true;
                    else
                        success = true;
                }
                else
                    failure = true;

            }
        }
    } while (not success and not failure and std::getline(file, piece, ';'));
    file.close();
    if (damage < INITIAL_HEALTH)
        failure = true;
    if (failure)
    {
        while (not level.empty())
            level.pop();
    }
    return success;
}

void Manager::prepareLevelNames(float window_height)
{
    m_texts.clear();
    float y_coord = MARGIN[TOP];
    for (const std::string& level : m_levels_dictionary | std::ranges::views::keys)
    {
        m_texts.emplace_back(level, m_arial);
        m_texts.back().setPosition(MARGIN[LEFT], y_coord);
        y_coord += TEXT_SIZE;
    }
    if (m_texts.size() * TEXT_SIZE > window_height)
    {
        m_overflow = true;
        m_shown_count = static_cast<unsigned int>((window_height - MARGIN[TOP] - MARGIN[BOTTOM]) / TEXT_SIZE);
    }
    else
    {
        m_overflow = false;
        m_shown_count = m_texts.size();
    }
}

void Manager::loadFont()
{
    if (not m_arial.loadFromFile(FONT_FILE))
        throw Error(Problem::FileError);
}

const sf::Font& Manager::shareFont()
{
    return m_arial;
}

void Manager::checkMaps()
{
    std::filesystem::path source(MAPS_DIR);
    if (not std::filesystem::exists(source))
        throw Error(Problem::FileError);

    auto analyse = [](const std::filesystem::path& file) -> std::pair<bool, std::string>
    {
        Graph graph;
        std::string name;
        bool result = extractFile(file, name, graph);
        if (result)
            result = findSimpleErrors(graph);
        if (result)
            result = checkConnectedness(graph);
        return std::make_pair(result, name);
    };
    std::vector<std::future<std::pair<bool, std::string>>> results;
    std::vector<std::filesystem::path> paths;
    std::vector<std::thread> threads;

    for (const auto& file : std::filesystem::directory_iterator(source))
    {
        if (file.path().extension() == MAP_EXTENSION)
        {
            std::packaged_task<std::pair<bool, std::string>(const std::filesystem::path&)> task(analyse);
            results.emplace_back(task.get_future());
            threads.emplace_back(std::move(task), file.path());
            paths.emplace_back(file.path());
        }
    }

    for (int i = 0; i < results.size(); ++i)
    {
        threads[i].join();
        auto pair = results[i].get();
        if (pair.first)
            m_maps_dictionary[pair.second] = paths[i];
    }

    if (m_maps_dictionary.empty())
        throw Error(Problem::NoSources);
}

void Manager::loadMap(sf::RenderWindow& window, std::string& map_name)
{
    prepareMapNames(static_cast<float>(window.getSize().y));
    map_name = selectText(window, "Please select a map (click):");
    std::filesystem::path source = m_maps_dictionary.at(map_name);
    if (not std::filesystem::exists(source))
        throw Error(Problem::FileError);
    Graph graph;
    bool result = extractFile(source, map_name, graph);
    if (result)
        result = findSimpleErrors(graph);
    if (result)
        result = checkConnectedness(graph);
    World& world = World::getInstance();
    if (result)
    {
        refactorGraph(graph);
        world.loadMap(graph);
    }
    graph.body.clear();
}

void Manager::checkLevels()
{
    std::filesystem::path source(LEVELS_DIR);
    if (not std::filesystem::exists(source))
        throw Error(Problem::FileError);
    std::string level_name;
    for (const auto& file : std::filesystem::directory_iterator(source))
    {
        if (file.path().extension() == LEVEL_EXTENSION)
        {
            if (validLevel(file.path(), level_name))
                m_levels_dictionary[level_name] = file.path();
        }
    }
    if (m_levels_dictionary.empty())
        throw Error(Problem::NoSources);
}

void Manager::loadLevel(sf::RenderWindow& window, Level& level, std::string& level_name)
{
    window.clear();
    prepareLevelNames(static_cast<float>(window.getSize().y));
    level_name = selectText(window, "Please select a level of difficulty (click):");
    std::filesystem::path path = m_levels_dictionary.at(level_name);
    if (not std::filesystem::exists(path))
        throw Error(Problem::FileError);
    if (not readLevel(path, level))
        throw Error(Problem::FileError);
}

void Manager::readEntitiesData()
{
    std::filesystem::path source(ENTITIES_DIR);
    source /= ENTITIES_SOURCE;
    if (not std::filesystem::exists(source))
        throw Error(Problem::NoSources);
    std::regex record(R"((?:\s*(?:#.*\n\s*)?)*)"
        R"*(\s*(\w+)[\t ]+)*" // Capturing Group \1 -> label
        R"*(((?:\d+(?:\.\d*)?)|(?:\.\d+))[\t ]+)*" // CG \2 -> speed
        R"*((\d+)[\t ]+(\d+)[\t ]+)*" // CG \3 -> life, CG \4 -> force
        R"*((\d+)[\t ]+)*" // CG \5 -> prize
        R"*((\d+)[\t ]+(\d+)[\t ]+)*" // CG \6 -> width, CG \7 -> height
        R"*(((?:\d+(?:\.\d*)?)|(?:\.\d+))[\t ]+)*" // CG \8 -> scale
        R"*("(.+)")*"); // CG \9 -> source file
    std::smatch result;
    std::ifstream file;
    std::string piece;
    std::filesystem::path texture_path;
    int count = 0;
    file.open(source);
    if (file.bad())
    {
        m_entities_dictionary.clear();
        m_entities_data.clear();
        throw Error(Problem::FileError);
    }
    while (std::getline(file, piece, ';'))
    {
        if (not std::regex_match(piece, result, record))
            break;
        std::string label(result[1].first, result[1].second);
        texture_path = std::string(result[9].first, result[9].second);
        if (not std::filesystem::exists(texture_path))
        {
            file.close();
            m_entities_dictionary.clear();
            m_entities_data.clear();
            throw Error(Problem::FileError);
        }
        if (m_entities_dictionary.find(label) != m_entities_dictionary.end())
        {
            file.close();
            m_entities_dictionary.clear();
            m_entities_data.clear();
            throw Error(Problem::FileError);
        }
        m_entities_dictionary[label] = count++;
        m_entities_data.emplace_back();
        m_entities_data.back().speed = std::stof(std::string(result[2].first, result[2].second));
        m_entities_data.back().health = std::stoi(std::string(result[3].first, result[3].second));
        m_entities_data.back().force = std::stoi(std::string(result[4].first, result[4].second));
        m_entities_data.back().prize = std::stoi(std::string(result[5].first, result[5].second));
        m_entities_data.back().dimensions.x = std::stof(std::string(result[6].first, result[6].second));
        m_entities_data.back().dimensions.y = std::stof(std::string(result[7].first, result[7].second));
        m_entities_data.back().scale = std::stof(std::string(result[8].first, result[8].second));
        m_entities_data.back().texture_path = texture_path;
    }
    file.close();
    for (auto it = m_entities_data.begin(); it != m_entities_data.end(); ++it)
    {
        if (not it->loadTexture())
        {
            m_entities_dictionary.clear();
            m_entities_data.clear();
            throw Error(Problem::FileError);
        }
    }
}

const EntityRecord& Manager::getEntityRecord(int index)
{
    try
    {
        return m_entities_data.at(index);
    }
    catch (...)
    {
        throw Error(Problem::OutOfRange);
    }
}

void Manager::readDefencesData()
{
    std::filesystem::path source(DEFENCES_DIR);
    source /= DEFENCES_SOURCE;
    if (not std::filesystem::exists(source))
        throw Error(Problem::NoSources);
    std::regex record(R"((?:\s*(?:#.*\n\s*)?)*)"
        R"*(\s*(\w+):[\t ]+)*" // CG \1 -> label
        R"*(((?:\d+(?:\.\d*)?)|(?:\.\d+))[\t ]+)*" // CG \2 -> radius
        R"*((\d+)[\t ]+(\d+)[\t ]+)*" // CG \3 -> period, CG \4 -> force
        R"*((\d+)[\t ]+(\d+)[\t ]+)*" // CG \5 -> hits, CG \6 -> cost, 
        R"*((\d+)[\t ]+(\d+)[\t ]+)*" // CG \7 -> width, CG \8 -> height
        R"*(((?:\d+(?:\.\d*)?)|(?:\.\d+))[\t ]+)*" // CG \9 -> scale
        R"*("(.+)")*" // CG \10 ->source file
    );
    std::smatch result;
    std::ifstream file;
    std::string piece;
    std::filesystem::path texture_path;
    int i = 0;
    file.open(source);
    if (file.bad())
    {
        m_defences_data.clear();
        throw Error(Problem::FileError);
    }
    while (std::getline(file, piece, ';'))
    {
        if (not std::regex_match(piece, result, record))
            break;
        std::string label(result[1].first, result[1].second);
        if (label != LABELS[i])
        {
            m_defences_data.clear();
            throw Error(Problem::FileError);
        }
        texture_path = std::string(result[10].first, result[10].second);
        if (not std::filesystem::exists(texture_path))
        {
            file.close();
            m_defences_data.clear();
            throw Error(Problem::FileError);
        }
        DefenceRecord record;
        record.radius = std::stof(std::string(result[2].first, result[2].second));
        record.period = std::stoi(std::string(result[3].first, result[3].second));
        record.force = std::stoi(std::string(result[4].first, result[4].second));
        record.hits = std::stoi(std::string(result[5].first, result[5].second));
        record.cost = std::stoi(std::string(result[6].first, result[6].second));
        record.dimensions.x = std::stof(std::string(result[7].first, result[7].second));
        record.dimensions.y = std::stof(std::string(result[8].first, result[8].second));
        record.scale = std::stof(std::string(result[9].first, result[9].second));
        record.texture_path = texture_path;
        m_defences_data[static_cast<DefenceType>(i)] = record;
        ++i;
    }
    file.close();
    for (auto it = m_defences_data.begin(); it != m_defences_data.end(); ++it)
    {
        if (not it->second.loadTexture())
        {
            m_defences_data.clear();
            throw Error(Problem::FileError);
        }
    }
}

DefenceRecord& Manager::getDefenceRecord(DefenceType type)
{
    try
    {
        m_defences_data.at(type);
    }
    catch (...)
    {
        throw Error(Problem::OutOfRange);
    }
}
