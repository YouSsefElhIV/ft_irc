#include "commands/CommandHandler.hpp"
#include "server/Server.hpp"

void CommandHandler::_Pass(Client &client, std::istringstream &iss) {
    std::string pass;
    iss >> pass;

    if (pass.empty()) {
        std::string msg = "461 " + client.getNick() + " PASS :Not enough parameters\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    if (pass != _server->getPassword()) {
        std::string msg = "464 " + client.getNick() + " :Password incorrect\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    client.authenticate();

    std::cout << "PASS correct for Client FD: " << client.getFd() << std::endl;
}


void CommandHandler::_User(Client &client, std::istringstream &iss) {
    std::string user, servername, realname;
    if (!(iss >> user >> servername)) {
        std::string err = "461 " + client.getNick() + " USER :Not enough parameters\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
    std::getline(iss, realname);
    if (!realname.empty() && realname[0] == ' ') realname.erase(0,1);
    client.setUser(user, realname, client.getHostName(), servername);

    // ************************* anchouf wach nick already set -> register client************************* 
    if (client.hasNick()) {
        client.setRegistered();
        std::string welcome = "001 " + client.getNick() + " :Welcome to the IRC network " + client.getNick() + "\r\n";
        send(client.getFd(), welcome.c_str(), welcome.size(), 0);

        std::string yourhost = "002 " + client.getNick() + " :Your host is " + client.getHostName() + ", running version 1.0\r\n";
        send(client.getFd(), yourhost.c_str(), yourhost.size(), 0);
    }
}

bool CommandHandler::isValidNick(const std::string &nick) {
    if (nick.empty() || nick.size() > 9) return false;
    if (!isalpha(nick[0])) return false;

    for (size_t i = 1; i < nick.size(); i++) {
        char c = nick[i];
        if (!(isalnum(c) || strchr("-[]\\`^{}|", c))) {
            return false;
        }
    }
    return true;
}


void CommandHandler::_NickName(Client &client, std::istringstream &iss) {
    std::string nick;
    iss >> nick;

    // ************** no nickname **************
    if (nick.empty()) {
        std::string msg = ":server 431 * :No nickname given\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // ************** check extra parameters **************
    std::string extra;
    if (iss >> extra) {
        std::string msg = ":server 432 * " + nick + " :Erroneous nickname\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // ************** validate nickname **************
    if (!isValidNick(nick)) {
        std::string msg = ":server 432 * " + nick + " :Erroneous nickname\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // ************** check if already exists **************
    if (_server->nickExists(nick)) {
        std::string msg = ":server 433 * " + nick + " :Nickname is already in use\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // ************** remove old nick if exists **************
    if (client.hasNick()) {
        _server->removeNick(client.getNick());
    }

    // ************** set new nickname **************
    client.setNick(nick);
    _server->addNick(nick, &client);

    // ************** register if USER already set **************
    if (client.hasNick() && client.hasUser()) {
        client.setRegistered();

        // ************** send RFC 1459 numeric replies **************
        std::string welcome = "001 " + client.getNick() + " :Welcome to the IRC network " + client.getNick() + "\r\n";
        send(client.getFd(), welcome.c_str(), welcome.size(), 0);

        std::string yourhost = "002 " + client.getNick() + " :Your host is " + client.getHostName() + ", running version 1.0\r\n";
        send(client.getFd(), yourhost.c_str(), yourhost.size(), 0);
    }
}



void CommandHandler::_Privmsg(Client &client, std::istringstream &iss, std::string which) {
    std::string target;
    iss >> target;

    std::string message;

    if (!iss.eof()) {
        std::string rest;
        std::getline(iss, rest);

        if (!rest.empty() && rest[0] == ' ')
            rest.erase(0, 1);

        if (!rest.empty() && rest[0] == ':') {    //********************************* case Dyal wla kante ':' *********************************
            message = rest.substr(1);
        } else {                                  //********************************* case Dyal wla ma kante ':' nktabe lklma lawela *********************************
            std::istringstream tmp(rest);
            tmp >> message;
            // message = rest;
        }
    }
    if (target.empty()) {
        std::string err = ":server 411 " + client.getNick() + " " + which + " :No recipient given\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
    if (message.empty()) {
        std::string err = ":server 412 " + client.getNick() + " " + which + " :No text to send\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }

    //********************************* Channel case *********************************
    if (target[0] == '#') {
        Channel* channel = _server->getChannelByName(target);
        if (!channel) {
            std::string err = "403 " + client.getNick() + " " + target + " :No such channel\r\n";
            send(client.getFd(), err.c_str(), err.size(), 0);
            return;
        }

        if (!_server->PartOfThatChannel(client.getFd(), channel->getName())) {
            std::string err = "404 " + client.getNick() + " " + target + " :Cannot send to channel\r\n";
            send(client.getFd(), err.c_str(), err.size(), 0);
            return;
        }

        const std::map<int, bool> &members = channel->getMembers();
        for (std::map<int, bool>::const_iterator it = members.begin(); it != members.end(); ++it) {
            if (it->first == client.getFd())
                continue;
            std::string fullMsg = ":" + client.getNick() + " " + which + " " + target + " :" + message + "\r\n";
            send(it->first, fullMsg.c_str(), fullMsg.size(), 0);
        }
    }
    //********************************* User case *********************************
    else {
        Client* dest = NULL;
        
        if (_server->isBotEnabled() && target == _server->getBotNick())
            dest = _server->getBotClient();
        else
            dest = _server->getClientByNick(target);
        
        if (!dest) {
            std::string err = ":server 4011 " + client.getNick() + " " + target + " :No such nick\r\n";
            send(client.getFd(), err.c_str(), err.size(), 0);
            return;
        }
        //************** Call bot if enabled **************
        if (_server->isBotEnabled() && target == _server->getBotNick()) {
            _server->handleBotCommand(&client, message);
            return ;
        }
        std::string fullMsg = ":" + client.getNick() + " " + which + " " + target + " :" + message + "\r\n";
        send(dest->getFd(), fullMsg.c_str(), fullMsg.size(), 0);
    }
}
