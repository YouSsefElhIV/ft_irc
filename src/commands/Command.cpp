#include "commands/CommandHandler.hpp"
#include "server/Server.hpp"

CommandHandler::CommandHandler(Server *server) : _server(server) {}


void CommandHandler::handleCommand(Server &server, Client &client, const std::string &line) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "PASS")
    {
        _Pass(client, iss);
    }
    else if (cmd == "NICK")
    {
        if (!client.isAuthenticated()) {
            send(client.getFd(), "451 :You are not registered\r\n", 30, 0);
            return;
        }
        _NickName(client, iss);
    }
    else if (cmd == "USER")
    {
        if (!client.isAuthenticated()) {
            send(client.getFd(), "451 :You are not registered\r\n", 30, 0);
            return;
        }
        _User(client, iss);
    }
    else if (cmd == "PRIVMSG")
    {
        if (!client.isRegistered()) {
            send(client.getFd(), "451 :You are not registered\r\n", 30, 0);
            return;
        }
        _Privmsg(client, iss, "PRIVMSG");
    }
    else {
        if (cmd == "HELP")
        {
            Help(client);
            return ;
        }
        if (!client.isRegistered()) {
            send(client.getFd(), "Client not registered yet\n", 27 ,0);
            return;
        }
        else if (cmd == "KICK" || cmd == "INVITE" || cmd == "TOPIC" || cmd == "MODE")
            executeOpCommand(server, client, iss, cmd);
        else
            executeCommand(server, client, iss, cmd);
    }
}
