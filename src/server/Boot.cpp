#include "server/Server.hpp"

void Server::initializeBot()
{
    if (_botEnabled)
        return;
    _botClient = new Client();
    _botClient->setNick("KanekiBot");
    _botClient->setRegistered();
    _clientsByNick["KanekiBot"] = _botClient;

    _botPunchline.push_back("lli fat mat 🔥");
    _botPunchline.push_back("lli ma3ndou 3asal kaykoun 9arsso 🐝");
    _botPunchline.push_back("dima kayn wa7ed 3ndou zidha 😂");
    _botPunchline.push_back("lli darha b yedou ghadi y7ss biha 💪");
    _botPunchline.push_back("m3a rassak tn3ess 🌙");

    _botEnabled = true;
    std::cout << "\033[34mBot 'KanekiBot' initialized and ready!\n\033[0m";
}

std::string Server::getRandomPunchline()
{
    if (_botPunchline.empty())
        return "I don't have a punchline right now... Sorry!";

    srand(time(NULL));
    size_t index = rand() % _botPunchline.size();
    return _botPunchline[index];
}

void Server::handleBotCommand(Client *client, const std::string &message)
{
    if (!_botEnabled)
    {
        std::cout << "!_botEnabled\n";
        return;
    }

    std::string response;

    if (message == "!punchline")
    {
        std::string joke = getRandomPunchline();
        response = ":KanekiBot!bot@127.0.0.1 PRIVMSG " + client->getNick() + " :" + joke + "\r\n";
    }
    else if (message == "!help")
    {
        response = ":KanekiBot!bot@127.0.0.1 PRIVMSG " + client->getNick() + " :Commands: !punchline - !help - !time\r\n";
    }
    else if (message == "!time")
    {
        time_t now;
        time(&now);
        std::string timeStr = ctime(&now);
        if (!timeStr.empty() && timeStr[timeStr.size() - 1] == '\n')
            timeStr.erase(timeStr.size() - 1, 1);
        response = ":KanekiBot!bot@127.0.0.1 PRIVMSG " + client->getNick() + " :Current time: " + timeStr + "\r\n";
    }
    else
    {
        response = ":KanekiBot!bot@127.0.0.1 PRIVMSG " + client->getNick() + " :SORRY, I don't understand '" + message + "'. Type !help for help.\r\n";
    }
    int fd = client->getFd();
    send(fd, response.c_str(), response.size(), 0);
}
