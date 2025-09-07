#include "server/Client.hpp"


Client::Client() : _fd(-1), _registered(false), _authenticated(false), permission(false) {}
Client::Client(int fd) : _fd(fd), _registered(false), _authenticated(false), permission(false) {}

Client::~Client() {}

int Client::getFd() const { return _fd; }

void Client::appendBuffer(const std::string &data)
{
    _buffer += data;
}

std::string &Client::getBuffer()
{
    return _buffer;
}

bool    Client::isRegistered() const { return _registered; }
void    Client::setPermission(const bool &value) { this->permission = value; }

void Client::setNick(const std::string &nick)
{
    _nickname = nick;
    if (!_username.empty())
        _registered = true;
}

void Client::setUser(const std::string &user, const std::string &realname, const std::string &hostname, const std::string &servername)
{
    _username = user;
    _realname = realname;
    _hostname = hostname;
    _servername = servername;
    if (!_nickname.empty())
        _registered = true;
}
void Client::setHostname(const std::string &hostname) {
    _hostname = hostname;
}
std::string Client::getHostName() const { return _hostname; }
std::string Client::getServeName() const { return _servername; }
std::string Client::getNick() const { return _nickname; }
std::string Client::getUser() const { return _username; }
bool        Client::getPermission() const { return permission; }
