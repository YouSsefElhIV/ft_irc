#include "commands/CommandHandler.hpp"
#include "server/Server.hpp"

void    CommandHandler::Broadcast_stat(Client& client, Channel &channel, std::string msg)
{
	(void)client;
	std::map<int, bool> members = channel.getMembers();
	std::map<int, bool>::iterator members_it = members.begin();

	while (members_it != members.end())
	{
		send(members_it->first, msg.c_str(), msg.size(), 0);
		members_it++;
	}
}

int	is_digits(std::string num)
{
	for (size_t i = 0; i < num.size(); i++)
	{
		if (num[i] < '0' || num[i] > '9')
			return (1);
	}
	return (0);
}

void CommandHandler::Mode(Server &server, Client &client, std::istringstream &iss)
{
	std::string name;
	std::string flag;
	std::string arg;

	iss >> name;
	iss >> flag;
	iss >> arg;
	if (name.size() == 0 || flag.size() == 0)
	{
		name = "MODE";
		flag = ERR_NEEDMOREPARAMS(name);
		send(client.getFd(), flag.c_str(), flag.size() ,0);
		return ;
	}
	if (name[0] != '#')
	{
		flag = ERR_NOSUCHCHANNEL(name);
		send(client.getFd(), flag.c_str(), flag.size() ,0);
		return ;
	}
	if ((flag[0] != '-' && flag[0] != '+'))
	{
		name = ERR_UMODEUNKNOWNFLAG(flag);
		send(client.getFd(), name.c_str(), name.size() ,0);
		return ;
	}
	Channel *channel = server.getChannelByName(name);
	if (!channel)
	{
		flag = ERR_NOSUCHCHANNEL(name);
		send(client.getFd(), flag.c_str(), flag.size() ,0);
		return ;
	}
	std::map<int, bool> members = channel->getMembers();
	std::map<int, bool>::iterator members_it = members.begin();
	while (members_it != members.end())
	{
		if (members_it->first == client.getFd())
			break ;
		members_it++;
	}
	if (members_it == members.end())
	{
        flag = ERR_NOTONCHANNEL(client.getNick(), name);
        send(client.getFd(), flag.c_str(), flag.size() ,0);
        return ; 
	}
	if (members_it->second == false)
	{
            flag = ERR_CHANOPRIVSNEEDED(channel->getName());
            send(client.getFd(), flag.c_str(),flag.size() ,0);
            return ;
	}
	if (flag == "+i")
	{
		channel->setInviteOnly(true);
		flag = RPL_MODE_OP_CHANGE(client.getNick(), channel->getName(), "+i", name);
		Broadcast_stat(client, *channel, flag);
	}
	else if (flag == "-i")
	{
		channel->setInviteOnly(false);
		flag = RPL_MODE_OP_CHANGE(client.getNick(), channel->getName(), "-i", name);
		Broadcast_stat(client, *channel, flag);
	}
	else if (flag == "+t")
	{
	    channel->setTopic(true);
		flag = RPL_MODE_OP_CHANGE(client.getNick(), channel->getName(), "+t", name);
		Broadcast_stat(client, *channel, flag);
	}
	else if (flag == "-t")
	{
	    channel->setTopic(false);
		flag = RPL_MODE_OP_CHANGE(client.getNick(), channel->getName(), "-t", name);
		Broadcast_stat(client, *channel, flag);
	}
	else if (flag == "+k")
	{
		if (arg.size() != 0)
		{
			channel->setPassword(arg);
			channel->setNoPassword(false);
			flag = RPL_MODE_SET_KEY(client.getNick(), (channel->getName()), arg);
			Broadcast_stat(client, *channel, flag);
		}
		else
		{
			name = "MODE";
			flag = ERR_NEEDMOREPARAMS(name);
			send(client.getFd(), flag.c_str(), flag.size() ,0);
		}
	}
	else if (flag == "-k")
	{
		if (arg.size() != 0)
		{
			name = "MODE";
			flag = ERR_NEEDMOREPARAMS(name);
			send(client.getFd(), flag.c_str(), flag.size() ,0);
		}
		else
		{
			std::string pass = "";
			channel->setNoPassword(true);
			channel->setPassword(pass);
			flag = RPL_MODE_UNSET_KEY(client.getNick(), channel->getName());
			Broadcast_stat(client, *channel, flag);
		}
	}
	else if (flag == "+o")
	{
		if (arg.size() == 0)
		{
			name = "MODE";
			flag = ERR_NEEDMOREPARAMS(name);
			send(client.getFd(), flag.c_str(), flag.size() ,0);
			return ;
		}
		std::map<int, bool> &members = channel->getMembers();
		std::map<int, bool>::iterator it = members.begin();
		Client *target = server.getClientByNick(arg);
		if (!target)
		{
			flag = ERR_USERNOTINCHANNEL(arg, channel->getName());
			send(client.getFd(), flag.c_str(), flag.size(), 0);
			return ;
		}
		while (it != members.end())
		{
			if (it->first == target->getFd())
				break ;
			it++;
		}
		members[it->first] = true;
		flag = RPL_MODE_OP_CHANGE(client.getNick(), channel->getName(), "+o", name);
		Broadcast_stat(client, *channel, flag);
	}
	else if (flag == "-o")
	{
		if (arg.size() == 0)
		{
			name = "MODE";
			flag = ERR_NEEDMOREPARAMS(name);
			send(client.getFd(), flag.c_str(), flag.size() ,0);
			return ;
		}
		std::map<int, bool> &members = channel->getMembers();
		std::map<int, bool>::iterator it = members.begin();
		Client *target = server.getClientByUser(arg);
		if (!target)
		{
			flag = ERR_USERNOTINCHANNEL(arg, channel->getName());
			send(client.getFd(), flag.c_str(), flag.size(), 0);
			return ;
		}
		while (it != members.end())
		{
			if (it->first == target->getFd())
				break ;
			it++;
		}
		members[it->first] = false;
		flag = RPL_MODE_OP_CHANGE(client.getNick(), channel->getName(), "-o", name);
		Broadcast_stat(client, *channel, flag);
	}
	else if (flag == "+l")
	{
		if (arg.size() == 0)
		{
			name = "MODE";
			flag = ERR_NEEDMOREPARAMS(name);
			send(client.getFd(), flag.c_str(), flag.size() ,0);
			return ;
		}
		if (is_digits(arg) == 1)
		{
			flag = ERR_INVALIDLIMIT(channel->getName());
			send(client.getFd(), flag.c_str(), flag.size(), 0);
			return ;
		}
		if (static_cast<int>(members.size()) > std::atoi(arg.c_str()))
		{
			flag = ERR_LIMITTOOSMALL(channel->getName());
			send(client.getFd(), flag.c_str(), flag.size(), 0);
			return ;
		}
		channel->setThereIsLimit(true);
		channel->setLimit(std::atoi(arg.c_str()));
		flag = RPL_MODE_OP_CHANGE(client.getNick(), channel->getName(), "+l", name);
		Broadcast_stat(client, *channel, flag);
	}
	else if (flag == "-l")
	{
		if (arg.size() != 0)
		{
			name = "MODE";
			flag = ERR_NEEDMOREPARAMS(name);
			send(client.getFd(), flag.c_str(), flag.size() ,0);
			return ;
		}
		channel->setThereIsLimit(false);
		flag = RPL_MODE_OP_CHANGE(client.getNick(), channel->getName(), "-l", name);
		Broadcast_stat(client, *channel, flag);
	}
	else
	{
		name = ERR_UMODEUNKNOWNFLAG(flag);
		send(client.getFd(), name.c_str(), name.size() ,0);
	}
}