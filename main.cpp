#include "Server.hpp"

int main(int argc, char **argv)
{
	Server s;
	try
	{
		s.setWebErrors();
		std::cout << "404 route: " << s.getWebError(404) << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}