#include "server/Server.hpp"

bool Signal = false;

void SignalHandler(int signum)
{
	(void)signum;
    std::cout << std::endl << "\033[34mSignal Received!\033[0m" << std::endl;
	Signal = true;
}
int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(av[1]);
    if (port <= 1024 || port > 65535)
    {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }

    std::string password = av[2];

    try
    {
        Server server(port, password);
        signal(SIGINT, SignalHandler);
		signal(SIGQUIT, SignalHandler);
        server.initializeBot();
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "\033[33mThe Server Closed!\033[0m" << std::endl;
    return 0;
}