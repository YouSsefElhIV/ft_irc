#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include "server/Client.hpp"
#include <string>
#include <map>

#define PREFIX ":IRCServer(YouSsef/Kanekki) "
#define POSTFIX "\r\n" 

//Mode 
#define ERR_UMODEUNKNOWNFLAG(mode)     PREFIX "501 MODE " + mode + " :Unknown MODE flag" POSTFIX
//Mode with K
#define RPL_MODE_SET_KEY(setter, channel, key) ":" + setter + " MODE " + channel + " +k " + key + POSTFIX
#define RPL_MODE_UNSET_KEY(setter, channel)     ":" + setter + " MODE " + channel + " -k" + POSTFIX
//Mode for O && L
#define RPL_MODE_OP_CHANGE(sender, channel, flag, target) ":" + sender + " MODE " + channel + " " + flag + " " + target + "\r\n"
#define ERR_INVALIDLIMIT(channel) ":server 696 " + channel + " :Invalid limit parameter\r\n"
#define ERR_LIMITTOOSMALL(channel) ":server 696 " + channel + " :Limit is lower than channel user count\r\n"


//Oper 
#define ERR_NEEDMOREPARAMS(command)						PREFIX "461 " + command + " :Not enough parameters" POSTFIX  
#define RPL_YOUREOPER(nick)                PREFIX "381 " + nick + " :You are now an IRC operator" POSTFIX

//Channel 
#define ERR_NOSUCHCHANNEL(channel)						PREFIX "403 " + channel + " :No such channel" POSTFIX
#define ERR_NOTONCHANNEL(target, channel)	   			PREFIX "442 " + target + " " + channel + " :You're not on that channel" POSTFIX 
#define ERR_CHANOPRIVSNEEDED(channel)					PREFIX "482 " + channel + " :You're not channel operator" POSTFIX
#define RPL_NOTOPIC(sender, channel)					PREFIX "331 " + channel + " :No topic is set" + POSTFIX
#define RPL_TOPIC(sender, channel, topic)				PREFIX "332 " + channel + " :" + topic + POSTFIX    

//Quit 
#define RPL_QUIT(nick, user, host, message) ":" + nick + "!" + user + "@" + host + " QUIT :" + message + "\r\n"

//Join 
#define RPL_JOIN(nick, user, host, channel) ":" + nick + "!" + user + "@" + host + " JOIN " + channel + POSTFIX
#define RPL_JOINMSG(nick, user, host, channel) ":" + nick + "!" + user + "@" + host + " JOIN :" + channel + POSTFIX
#define ERR_CHANNELISFULL(client, channel)				PREFIX " 471 " + client + " " + channel + " :Cannot join channel (+l)" POSTFIX
#define ERR_INVITEONLYCHAN(client, channel)				PREFIX " 473 " + client + " " + channel + " :Cannot join channel (+i)" POSTFIX

//Invite 
#define RPL_INVITING(nickname, targnick, targchan)  	": 341 " + nickname + " " + targnick + " " + targchan + POSTFIX
#define RPL_INVITE(sender, target, channel)				":" + sender + " INVITE " + target + " " + channel + POSTFIX

//Kick 
#define RPL_KICK_BROADCAST(channel, target, reason)  ":IRCServer(YouSsef/Kanekki) KICK " + channel + " " + target + " :" + reason + "\r\n"

//:server.name 481 <nick> <command> :Permission Denied- You're not an IRC operator

//control flow
#define RPL_WELCOME(sender, msg)               			PREFIX "001 " + sender + " : " + msg + POSTFIX 
#define ERR_USERNOTANOP(nick, command)                      PREFIX "481 " + nick + " " + command + " :Permission Denied- You're not an IRC operator" POSTFIX
#define ERR_USERNOTINCHANNEL(user, channel) 			PREFIX "441 " + user + " " + channel + " :They aren't on that channel" POSTFIX 
#define ERR_UNKNOWNCOMMAND(command)						PREFIX "421 " + command + " :Unknown command" POSTFIX 
#define ERR_NOSUCHNICK(nick)							PREFIX "401 " + nick + " :No such nick/channel" POSTFIX 
#define ERR_INCORPASS(nickname)							(": 464 " + nickname + " :Password incorrect !" + POSTFIX )

class Server;
class Channel;

class CommandHandler
{
    private:
        Server *_server;
    public:  
        CommandHandler(Server *server);
        void handleCommand(Server &server, Client &client, const std::string &line);
        void executeOpCommand(Server &server, Client &client, std::istringstream &iss, std::string &cmd);
        void executeCommand(Server &server, Client &client, std::istringstream &iss, std::string &cmd);
        void _Pass(Client &client, std::istringstream &iss);
        void _User(Client &client, std::istringstream &iss);
        void _NickName(Client &client, std::istringstream &iss);
        void _Privmsg(Client &client, std::istringstream &iss, std::string which);
        bool isValidNick(const std::string &nick);
        void Join(Server &server, Client &client, std::istringstream &iss);
        void Quit(Server &server, Client &client, std::istringstream &iss);
        void Oper(Server &server, Client &client, std::istringstream &iss);
        void Help(Client &client);
        void Kick(Server &server, Client &client, std::istringstream &iss);
        void Invite(Server &server, Client &client, std::istringstream &iss);
        void Topic(Server &server, Client &client, std::istringstream &iss);
        void Mode(Server &server, Client &client, std::istringstream &iss);
        void Nicks(Server &server, Client &client, std::istringstream &iss);
        void Users(Server &server, Client &client, std::istringstream &iss);
        void Broadcast_stat(Client& client, Channel &channel, std::string msg);
};

#endif
