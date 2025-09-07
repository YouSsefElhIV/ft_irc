#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <poll.h>
#include <netinet/in.h>
#include "Client.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <set> 
#include <cerrno>
#include <cstring>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include "Channel.hpp"
#include "commands/CommandHandler.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <fcntl.h>

extern bool Signal;

class Server
{
    private:
        int                                     _port;
        std::string                             _password;
        std::string                             OpPassword;
        std::string                             OpUsername;
        int                                     _serverSocket;
        std::vector<pollfd>                     _pollFds;
        std::map<int, Client>                   _clients;
        std::map<std::string, Channel>          _channels;
        CommandHandler                          _commandHandler;
        std::map<std::string, Client*>          _clientsByNick;
        Client*                                 _botClient;
        bool                                    _botEnabled;
        std::vector<std::string>                _botPunchline;
    public:
        Server(int port, const std::string& password);
        ~Server();
        void cleanup();
        void run();
        void setupSocket();
        void removeClient(int fd);
        void handleNewConnection();
        void handleClientMessage(int fd);
        Client* getClientByNick(const std::string &nick);
        Client* getClientByUser(const std::string &user);
        std::map<int, Client> &getClients();
        Channel* getChannelByName(const std::string &name);
        const std::string& getPassword() const { return _password; }
        const std::string& getOpPassword() const { return OpPassword; }
        const std::string& getOpUsername() const { return OpUsername; }
        std::map<std::string, Channel> &getChannels();
        void deleteChannel(std::string name);
        bool nickExists(const std::string &nick) const;
        void addNick(const std::string &nick, Client* client);
        void removeNick(const std::string &nick);
        int  lookForChannel(Client &client, std::string &name, std::string &password);
        void createChannel(Client &client, std::string &name, std::string &password);
        int  PartOfThatChannel(int clientFd, std::string channel_name);
        void initializeBot();
        std::string getRandomPunchline();
        void handleBotCommand(Client* client, const std::string& message);
        Client* getBotClient() const { return _botClient; }
        bool isBotEnabled() const { return _botEnabled; }
        std::string getBotNick() const { return _botClient ? _botClient->getNick() : ""; }
        void    exec_cmd(int fd);
};

#endif
