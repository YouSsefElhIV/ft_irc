#include "commands/CommandHandler.hpp"
#include "server/Server.hpp"

void    CommandHandler::executeOpCommand(Server &server, Client &client, std::istringstream &iss, std::string &cmd)
{
    if (cmd == "KICK")
        Kick(server, client, iss);
    else if (cmd == "INVITE")
        Invite(server, client, iss);
    else if (cmd == "TOPIC")
        Topic(server, client, iss);
    else if (cmd == "MODE")
        Mode(server, client, iss);
}

void    CommandHandler::executeCommand(Server &server, Client &client, std::istringstream &iss, std::string &cmd)
{
    if (cmd == "JOIN")
        Join(server, client, iss);
    else if (cmd == "QUIT")
        Quit(server, client, iss);
    else if (cmd == "OPER")
        Oper(server, client, iss);
    else if (cmd == "NICKS")
        Nicks(server, client, iss);
    else if (cmd == "USERS")
        Users(server, client, iss);
    else
    {
        std::string msg = ERR_UNKNOWNCOMMAND(cmd);
        send(client.getFd(), msg.c_str(), msg.size(), 0);
    }
}

void CommandHandler::Quit(Server &server, Client &client, std::istringstream &iss)
{
    std::map<std::string, Channel> &channels = server.getChannels();
    std::map<std::string, Channel>::iterator it = channels.begin();
    std::string msg;
    std::string word;
    iss >> word;
    if (word.size() > 0)
    {
        msg += word;
        while (iss >> word)
        {
            msg += word;
            msg += " ";
        }
    }
    if (msg.size() == 0 || (msg.size() > 0 && msg[0] != ':'))
    {
        msg = "QUIT";
        word = ERR_NEEDMOREPARAMS(msg);
        send(client.getFd(), word.c_str(), word.size(), 0);
        return ;
    }
    while (it != channels.end())
    {
        std::map<int, bool> &members = it->second.getMembers();
        std::map<int, bool>::iterator members_it = members.begin();
        while (members_it != members.end())
        {
            if (members_it->first == client.getFd())
            {
                for (members_it = members.begin(); members_it != members.end(); ++members_it) {
                    if (members_it->first != client.getFd())
                    {
                        msg = RPL_QUIT(client.getNick(), client.getUser(), client.getHostName(), msg);
                        send(members_it->first, msg.c_str(), msg.size(), 0);
                    }
                }
                members.erase(client.getFd());
                break ;
            }
            members_it++;
        }
        std::map<std::string, Channel>::iterator tmp = it;
        it++;
        if (members.size() == 0)
            server.deleteChannel(tmp->second.getName());
    }
    server.removeClient(client.getFd());
}

void CommandHandler::Nicks(Server &server, Client &client, std::istringstream &iss)
{
    std::string arg;
    iss >> arg;
    if (client.getPermission() == false)
    {
        arg = ERR_USERNOTANOP(client.getNick(), arg);
        send(client.getFd(), arg.c_str(), arg.size(), 0);
        return ;
    }
    if (arg.size() > 0)
    {
        arg = ERR_UNKNOWNCOMMAND(arg);
        send(client.getFd(), arg.c_str(), arg.size(), 0);
        return ;
    }
    std::map<int, Client> &clients = server.getClients();
    std::map<int, Client>::iterator it= clients.begin();
    send(client.getFd(), "Client's Nick List:\n", 21, 0);
    while (it != clients.end())
    {
        std::string msg = it->second.getNick();
        it++;
        if (it != clients.end())
            msg += "  ";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
    }
    send(client.getFd(), "\n", 1, 0);
}

void CommandHandler::Users(Server &server, Client &client, std::istringstream &iss)
{
    std::string arg;
    iss >> arg;
    if (client.getPermission() == false)
    {
        arg = ERR_USERNOTANOP(client.getNick(), arg);
        send(client.getFd(), arg.c_str(), arg.size(), 0);
        return ;
    }
    if (arg.size() > 0)
    {
        arg = ERR_UNKNOWNCOMMAND(arg);
        send(client.getFd(), arg.c_str(), arg.size(), 0);
        return ;
    }
    std::map<int, Client> &clients = server.getClients();
    std::map<int, Client>::iterator it= clients.begin();
    send(client.getFd(), "Client's Username List:\n", 25, 0);
    while (it != clients.end())
    {
        std::string msg = it->second.getUser();
        it++;
        if (it != clients.end())
            msg += "  ";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
    }
    send(client.getFd(), "\n", 1, 0);
}

void CommandHandler::Help(Client &client)
{
    std::string msg = "          CMD LIST:\n\nPASS:   Enter PASSWORD (PASS password)\n\
                            NICK:   Enter NICKNAME (NICK YourChoice)\n\
                            JOIN:   Makes the client join the channels in the comma-separated list <channels>, specifying the passwords, if needed, in the comma-separated list <keys>.(JOIN <channels> [<keys>])\n\
                            USER:   Enter a username (USER <username> <hostname> <servername> <realname>)\n\
                            PRIVMSG:To print a message (PRIVMSG receiverNick :YourMsg\n\
                            QUIT:  To get out of the server (QUIT <message>)\n\
                            OPER:    Authenticates a user as an IRC operator on that server/network (OPER <username> <password>)\n\n\
                                        OPERATORS CMD LIST\n\
                            NICKS:  To get all users nick, only Server Op can run this.\n\
                            USERS:  To get all users Username, only Server Op can run this.\n\
                            KICK:    To kick a user from a chat room (KICK <channel> <user> [<comment>])\n\
                            INVITE:  Invites <nickname> to the channel <channel>.[9] <channel> does not have to exist, but if it does, only members of the channel are allowed to invite other clients. \
If the channel mode i is set, only channel operators may invite other clients(INVITE <nickname> <channel>).\n\
                            TOPIC:   Allows the client to query or set the channel topic on <channel> (TOPIC <channel> [<topic>]).\n\
                            MODE:    The MODE command is dual-purpose. It can be used to set channel modes(MODE <channel> <flags> [<args>]).\n\
                                i: Set/remove Invite-only channel\n\
                                · t: Set/remove the restrictions of the TOPIC command to channel operators\n\
                                · k: Set/remove the channel key (password)\n\
                                · o: Give/take channel operator privilege\n\
                                · l: Set/remove the user limit to channel\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
}
