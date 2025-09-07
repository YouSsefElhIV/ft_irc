#include "server/Server.hpp"
#include "commands/CommandHandler.hpp"

int Server::lookForChannel(Client &client, std::string &name, std::string &password)
{
    std::map<std::string, Channel>::iterator it = this->_channels.begin();
    std::string msg;

    while (it != this->_channels.end())
    {
        if (it->second.getName() == name)
        {
            if (it->second.getPassword() == password)
            {
                if (it->second.getInviteOnly() == false || (it->second.getInviteOnly() == true && it->second.OnList(client.getFd()) == 0))
                {
                    if ((it->second.getThereIsLimit() == true && it->second.getLimit() == (int)it->second.getMembers().size()))
                    {
                        msg = ERR_CHANNELISFULL(client.getNick(), name);
                        send(client.getFd(), msg.c_str(), msg.size() ,0);
                    }
                    else
                    {
                        it->second.addMember(client.getFd(), false);
                        msg = RPL_JOIN(client.getNick(), client.getUser(), client.getHostName(), name);
                        send(client.getFd(), msg.c_str(), msg.size() ,0);
                        	std::map<int, bool> &members = it->second.getMembers();
	                        std::map<int, bool>::iterator members_it = members.begin();
	                        while (members_it != members.end())
	                        {
                                if (members_it->first != client.getFd())
                                {
                                    msg = RPL_JOINMSG(client.getNick(), client.getUser(), client.getHostName(), name);
	                        	    send(members_it->first, msg.c_str(), msg.size(), 0);
                                }
	                        	members_it++;
	                        }
                    }
                }
                else
                {
                    msg = ERR_INVITEONLYCHAN(client.getNick(), name);
                    send(client.getFd(), msg.c_str(), msg.size() ,0);
                }

            }
            else
            {
                msg = ERR_INCORPASS(client.getNick());
                send(client.getFd(), msg.c_str(), msg.size(), 0);
            }
            return (0);
        }
        it++;
    }
    return (1);
}

void    Server::createChannel(Client &client, std::string &name, std::string &password)
{
    Channel newChannel(name, password);
    this->_channels[name] = newChannel;
    this->_channels[name].addMember(client.getFd(), true);
    std::string msg = RPL_JOIN(client.getNick(), client.getUser(), client.getHostName(), name);
    send(client.getFd(), msg.c_str(), msg.size() ,0);
}

void CommandHandler::Join(Server &server, Client &client, std::istringstream &iss)
{
    std::string channel_name;
    std::string password;

    iss >> channel_name;
    iss >> password;
    if (channel_name.size() == 0)
    {
        password = "JOIN";
        channel_name = ERR_NEEDMOREPARAMS(password);
        send(client.getFd(), channel_name.c_str(), channel_name.size() ,0);
        return ;
    }
    else if (channel_name[0] != '#')
    {
        password = ERR_NOSUCHCHANNEL(channel_name);
        send(client.getFd(), password.c_str(), password.size() ,0);
        return ;
    }
    if (server.lookForChannel(client, channel_name, password) == 1)
        server.createChannel(client, channel_name, password);
}