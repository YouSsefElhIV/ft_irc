#include "server/Server.hpp"
#include "commands/CommandHandler.hpp"

Server::Server(int port, const std::string &password) : _port(port), _password(password), OpPassword("Admin"), OpUsername("Admin"), _serverSocket(-1), _commandHandler(this) {}

void Server::cleanup()
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        int fd = it->first;
        close(fd);
    }
    _clients.clear();
    if (_serverSocket > 0)
        close(_serverSocket);
    if (_botEnabled == true)
    {
        close(_botClient->getFd());
        delete _botClient;
        _botClient = NULL;
        _botEnabled = false;
    }
    std::cout << "\033[34mServer cleanup done, all sockets closed.\n\033[0m";
}

Server::~Server()
{
    cleanup();
}

void Server::setupSocket() {
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0)
        throw std::runtime_error("socket() failed");
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt() failed");
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);
    if (bind(_serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");
    int f = fcntl(_serverSocket, F_GETFL, 0);
    if (f == -1)
        throw std::runtime_error("fcntl() failed");
    if (fcntl(_serverSocket, F_SETFL, f | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl() failed");
    if (listen(_serverSocket, SOMAXCONN) < 0)
        throw std::runtime_error("listen() failed");
    pollfd pfd;
    pfd.fd = _serverSocket;
    pfd.events = POLLIN;
    _pollFds.push_back(pfd);
    std::cout << "\033[33mServer listening on port \033[0m" << _port << std::endl;
}

void Server::run() {
    setupSocket();

    while (!Signal) {
        int ret = poll(&_pollFds[0], _pollFds.size(), -1);
        if (ret < 0) {
            if (errno == EINTR)
                break;
            throw std::runtime_error("poll() failed");
        }
        size_t o = _pollFds.size();
        for (size_t i = 0; i < _pollFds.size(); i++) {
            if (_pollFds[i].revents & POLLIN) {
                if (_pollFds[i].fd == _serverSocket)
                    handleNewConnection();
                else
                {
                    o = _pollFds.size();
                    handleClientMessage(_pollFds[i].fd);
                    if (_pollFds.size() == o)
                        _pollFds[i].events = POLLOUT;
                }
            }
            else if (_pollFds[i].revents & POLLOUT)
            {
                o = _pollFds.size();
                exec_cmd(_pollFds[i].fd);
                if (_pollFds.size() == o)
                    _pollFds[i].events = POLLIN;
            }
        }
    }
    for (size_t i = 0; i < _pollFds.size(); i++) {
        close(_pollFds[i].fd);
    }
    _pollFds.clear();
}

void Server::handleNewConnection() {
    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &len);
    if (clientFd < 0) {
        std::cerr << "accept() failed\n";
        return;
    }
    int f = fcntl(_serverSocket, F_GETFL, 0);
    if (f == -1)
        throw std::runtime_error("fcntl() failed");
    if (fcntl(_serverSocket, F_SETFL, f | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl() failed");
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
    std::string clientIP = ipStr;
    Client c(clientFd);
    c.setHostname(clientIP);
    _clients.insert(std::make_pair(clientFd, c));
    pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    _pollFds.push_back(pfd);
    std::cout << "\033[32mNew client connected: FD " << clientFd << "\033[0m" << std::endl;
}

void Server::exec_cmd(int fd)
{
    std::map<int, Client>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return;
    Client &client = it->second;
    std::string &buffer = client.getBuffer();
        size_t pos = 0;
        while ((pos = client.getBuffer().find("\n")) != std::string::npos) {
            std::string line = buffer.substr(0, pos + 1);
            buffer = buffer.substr(pos + 1);
            while (!line.empty() && (line[line.size() - 1] == '\n' || line[line.size() - 1] == '\r'))
                line.resize(line.size() - 1);
            if (!line.empty()) {
                _commandHandler.handleCommand(*this, client, line);
                it = _clients.find(fd);
                if (it == _clients.end())
                    return;
            }
        }
}

void Server::handleClientMessage(int fd) {
    char tmp[512];
    ssize_t bytes = recv(fd, tmp, sizeof(tmp), 0);
    if (bytes <= 0) {
        std::cout << "\033[31mClient disconnected: FD " << fd << "\033[0m" << std::endl;
        std::string msg = "Clinet Disconected";
        std::istringstream iss(msg);
        Client c(fd);
        this->_commandHandler.Quit(*this, c, iss);
        removeClient(fd);
        return;
    }
    std::map<int, Client>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return;
    Client &client = it->second;
    client.appendBuffer(std::string(tmp, bytes));
    std::string &buffer = client.getBuffer();
    const size_t MAX_BUFFER_SIZE = 8192; // *********************** 8 KB per client***********************//
    if (buffer.size() > MAX_BUFFER_SIZE) {
        std::string msg = "ERROR :Input too long, disconnecting\r\n";
        send(fd, msg.c_str(), msg.size(), 0);
        std::cout << "Client Fd " << fd << " exceeded buffer limit. Disconnecting.\n";
        removeClient(fd);
        return;
    }
}

void Server::removeClient(int fd) {
    close(fd);
    _clients.erase(fd);
    for (size_t i = 0; i < _pollFds.size(); i++) {
        if (_pollFds[i].fd == fd) {
            _pollFds.erase(_pollFds.begin() + i);
            break;
        }
    }
}

bool Server::nickExists(const std::string &nick) const {
    for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.getNick() == nick)
            return true;
    }
    return false;
}

std::map<int, Client> &Server::getClients()
{
    return (this->_clients);
}

void Server::deleteChannel(std::string name)
{
    this->_channels.erase(name);
}

int Server::PartOfThatChannel(int fd, std::string channel)
{
    std::map<std::string, Channel>::iterator channel_it = this->_channels.begin();

    while (channel_it != this->_channels.end())
    {
        if (channel_it->second.getName() == channel)
        {
            std::map<int, bool> &clients = channel_it->second.getMembers();
            std::map<int, bool>::iterator client_it = clients.begin();
            while (client_it != clients.end())
            {
                if (client_it->first == fd)
                    return (1);
                client_it++;
            }
            break ;
        }
        channel_it++;
    }
    return (0);
}

Client* Server::getClientByNick(const std::string &nick) {
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.getNick() == nick)
            return &(it->second);
    }
    return NULL;
}

Client* Server::getClientByUser(const std::string &user) {
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.getUser() == user)
            return &(it->second);
    }
    return NULL;
}

Channel* Server::getChannelByName(const std::string &name) {
    std::map<std::string, Channel>::iterator it = _channels.find(name);
    if (it != _channels.end())
        return &(it->second);
    return NULL;
}

void Server::addNick(const std::string &nick, Client* client) {
    _clientsByNick[nick] = client;
}

void Server::removeNick(const std::string &nick) {
    _clientsByNick.erase(nick);
}

std::map<std::string, Channel> &Server::getChannels() {
    return (this->_channels);
}
