
# Ft_Irc

ft_irc is an irc server built in c++, it uses non blocking sockets API and handle multiple clients connection at the same time using the poll() system call.
## Intra Profile

- [yel-haya@me](https://profile.intra.42.fr/users/yel-haya)
- [iezzam@Teammate](https://profile.intra.42.fr/users/iezzam)


## Project Goal

- Undestand How sockets api work for network communication.

- Understand the client server architecture.

- Understand irc protocol and its commands.
## Pro Tip

- Teammate 1: implement The server and the authentication.
- Teammate 2: Make the Irc Commands:

        PRIVMSG: send a private or public message, to a user or a channel.
        JOIN: Join a channel.
        OPER(optional): Give a user admin privilages on the server.
        HELP(optional): To list the server commands and their info.
        NICKS(custom): List all nicknames on the server.
        USERS(custom): List all usernames on the server.

        (Commands for the channel).
        KICK: Kick a member from a channel.
        TOPIC: Set or output the channel topic.
        INVITE: Invite a user to a channel.
        MODE(with its 5 flags): change the channel settings (password, op memebers, etc...)
## Usage/Examples

        
        make && ./ircserv ValidPortNum Password.

That would run the server then You can connect to it using nc localhost portnum or limechat.

After connection You must enter first the server password, valid username then a valid nickname:
        
        PASS <password>.
        USER <username> <hostname> <servername> <realname>
        NICK <nickname>.
after connection You can run commands, for more information about the commands, use HELP.

## Sources

https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/

https://www.youtube.com/watch?v=gntyAFoZp-E

https://www.tutorialspoint.com/cplusplus/cpp_socket_programming.htm

https://www.linuxhowtos.org/C_C++/socket.htm

https://medium.com/@ahmadesekak/setting-up-a-server-using-c-sockets-124e404593c9
