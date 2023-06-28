#include "engine.h"
#include "error.h"
#include <iostream>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

int main()
{
	Engine& engine = Engine::getInstance();
	try
	{
		engine.prepare();
		while (engine.running())
		{
			engine.update();
			engine.render();
		}
		engine.finish();
		auto result = engine.getResult();
		switch (result)
		{
		case Result::Interrupt:
			std::cout << "Game was interrupted." << std::endl; break;
		case Result::Victory:
			std::cout << "You win!" << std::endl; break;
		case Result::Failure:
			std::cout << "You loose!" << std::endl; break;
		default:
			std::cout << "Unexpected occurrece." << std::endl;
		}
	}
	catch (Error err)
	{
		std::cout << "An error has been encountered:" << std::endl << std::endl
			<< "\t" << err.what() << std::endl << std::endl;
	}
	return 0;
}