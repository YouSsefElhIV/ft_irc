#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
    private:
        int                 _fd;
        std::string         _buffer;
        std::string         _nickname;
        std::string         _username;
        std::string         _hostname;
        std::string         _servername;
        std::string         _realname;
        bool                _registered;
        bool                _authenticated;
        bool                permission;
    public:
        Client();
        Client(int fd);
        ~Client();
        int getFd() const;
        void appendBuffer(const std::string &data);
        std::string& getBuffer();
        bool isRegistered() const;
        void setNick(const std::string &nick);
        void setPermission(const bool &value);
        void setUser(const std::string &user, const std::string &realname, const std::string &hostname, const std::string &servername);
        std::string getNick() const;
        std::string getUser() const;
        void setHostname(const std::string &hostname);
        std::string getHostName() const;
        std::string getServeName() const;
        bool        getPermission() const;
        bool isAuthenticated() const { return _authenticated; }
        void authenticate() { _authenticated = true; } 
        bool hasNick() const { return !_nickname.empty(); }
        bool hasUser() const { return !_username.empty(); }
        void setRegistered() { _registered = true; }
};

#endif
