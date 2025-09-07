#include "server/Server.hpp"
#include "commands/CommandHandler.hpp"

void CommandHandler::Kick(Server &server, Client &client, std::istringstream &iss)
{
    std::string msg;
    std::string channel;
    std::string name;
    std::string word;
    iss >> channel;
    iss >> name;
    while (iss >> word)
    {
        word += " ";
        msg += word;
    }
    if (channel.size() == 0 || name.size() == 0 || msg.size() == 0 || msg[0] != ':')
    {
        channel = "KICK";
        word = ERR_NEEDMOREPARAMS(channel);
        send(client.getFd(), word.c_str(), word.size(), 0);
        return ;
    }
    else if (channel[0] != '#')
    {
        word = ERR_NOSUCHCHANNEL(channel);
        send(client.getFd(), word.c_str(), word.size(), 0);
        return ;
    }
    else
    {
        msg = msg.substr(1);
        if (!server.getChannelByName(channel))
        {
            msg = ERR_NOSUCHCHANNEL(channel);
            send(client.getFd(), msg.c_str(), msg.size(), 0);
        }
        else
        {
            std::map<int, bool> &members = server.getChannelByName(channel)->getMembers();
            std::map<int, bool>::iterator members_it = members.begin();
            std::map<int, Client> &clients = server.getClients();
            std::map<int, Client>::iterator clients_it = clients.begin();
            Channel *channel_var = server.getChannelByName(channel);
            while (members_it != members.end())
            {
                if (members_it->first == client.getFd())
                {
                    if (members_it->second == true)
                    {
                        while (clients_it != clients.end())
                        {
                            if (clients_it->second.getNick() == name)
                            {
                                word = RPL_KICK_BROADCAST(channel, name, msg);
                                send(clients_it->first, word.c_str(), word.size(), 0);
                                channel_var->removeMember(clients_it->first);
                                if (name == client.getNick() && members.size() == 0)
                                {
                                    server.deleteChannel(channel_var->getName());
                                    return ;
                                }
                                if (members.size() > 0)
                                {
                                    word = RPL_KICK_BROADCAST(channel, name, msg);
                                    members = server.getChannelByName(channel)->getMembers();
                                    members_it = members.begin();
                                    while (members_it != members.end())
                                    {
                                        send(members_it->first, word.c_str(), word.size(), 0);
                                        members_it++;
                                    }
                                    return ;
                                }
                            }
                            clients_it++;
                        }
                    }
                    else
                    {
                        msg = ERR_CHANOPRIVSNEEDED(channel);
                        send(client.getFd(), msg.c_str(), msg.size(), 0);
                        return ;
                    }
                }
                members_it++;
            }
            msg = ERR_USERNOTINCHANNEL(name, channel);
            send(client.getFd(), msg.c_str(), msg.size(), 0);
        }
    }
}

void CommandHandler::Invite(Server &server, Client &client, std::istringstream &iss)
{
    std::string who;
    std::string where;
    iss >> who;
    iss >> where;

    if (who.size() == 0 || where.size() == 0)
    {
        where = "INVITE";
        who = ERR_NEEDMOREPARAMS(where);
        send(client.getFd(), who.c_str(), who.size() ,0);
        return ;
    }
    Client *target = server.getClientByNick(who);
    if (!target)
    {
        where = ERR_NOSUCHNICK(who);
        send(client.getFd(), where.c_str(), where.size() ,0);
        return ;
    }
    Channel *channel = server.getChannelByName(where);
    if (!channel)
    {
        who = ERR_NOSUCHCHANNEL(where);
        send(client.getFd(), who.c_str(), who.size() ,0);
        return ;
    }
    std::map<int , bool> &members= channel->getMembers();
    std::map<int , bool>::iterator members_it = members.begin();
    while (members_it != members.end())
    {
        if (members_it->first == client.getFd())
            break;
        members_it++;
    }
    if (members_it == members.end())
    {
        where = ERR_NOTONCHANNEL(who, where);
        send(client.getFd(), where.c_str(), where.size() ,0);
        return ; 
    }
    if (channel->getOnlyOpInvite() == true)
    {
        if (members_it->second == false)
        {
            who = ERR_CHANOPRIVSNEEDED(channel->getName());
            send(client.getFd(), who.c_str(),who.size() ,0);
            return ;
        }
    }
    channel->AddToList(target->getFd());
    who = RPL_INVITING(client.getNick(), target->getNick(), channel->getName());
    send(target->getFd(), who.c_str(), who.size(), 0);
}

void CommandHandler::Topic(Server &server, Client &client, std::istringstream &iss)
{
    std::string name;
    std::string msg;
    std::string word;
    iss >> name;
    while (iss >> word)
    {
        msg += word;
        msg += " ";

    }
    if (name.size() == 0)
    {
        name = "TOPIC";
        msg = ERR_NEEDMOREPARAMS(name);
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return ;
    }
    Channel *channel = server.getChannelByName(name);
    if (!channel)
    {
        msg = ERR_NOSUCHCHANNEL(name);
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return ;
    }
    std::map<int, bool> &members = channel->getMembers();
    std::map<int, bool>::iterator it = members.begin();
    while (it != members.end())
    {
        if (it->first == client.getFd())
            break ;
        it++;
    }
    if (it == members.end())
    {
        msg = ERR_NOTONCHANNEL(client.getNick(), channel->getName());
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return ;
    }
    if (msg.size() == 0)
    {
        std::string msg;
        if (channel->getTopicText().size() == 0)
            msg = RPL_NOTOPIC(client.getNick(), channel->getName());
        else
            msg = RPL_TOPIC(client.getNick(), channel->getName(), channel->getTopicText());
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return ;
    }
    if (msg[0] != ':')
    {
        name = "TOPIC";
        word = ERR_NEEDMOREPARAMS(name);
        send(client.getFd(), word.c_str(), word.size(), 0);
        return ;
    }
    if (channel->getTopic() == true)
    {
        if (members[client.getFd()] == false)
        {
            msg = ERR_CHANOPRIVSNEEDED(channel->getName());
            send(client.getFd(), msg.c_str(), msg.size(), 0);
            return ;
        }
    }
    channel->setTopicText(msg);
    msg = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHostName() + " TOPIC " + name + " :" + msg + "\n";
    Broadcast_stat(client, *channel, msg);
}

void CommandHandler::Oper(Server &server, Client &client, std::istringstream &iss)
{
    std::string msg;
    std::string arg1;
    std::string arg2;

    iss >> arg1;
    iss >> arg2;
    if (arg1.size() == 0 || arg2.size() == 0)
    {
        arg1 = "OPER";
        msg = ERR_NEEDMOREPARAMS(arg1);
        send(client.getFd(), msg.c_str(), msg.size(), 0);
    }
    else
    {
        if (server.getOpUsername() != arg1 || arg2 != server.getOpPassword())
            msg = ERR_INCORPASS(client.getNick());
        else
        {
            client.setPermission(true);
            msg = RPL_YOUREOPER(client.getNick());
        }
    }
    send(client.getFd(), msg.c_str(), msg.size(), 0);
}
