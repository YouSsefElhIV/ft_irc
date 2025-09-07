NAME		=		ircserv

CC			=		c++ 

CPPFLAGS	=		 -Wall -Werror -Wextra -g -fsanitize=address -std=c++98 -I./inc/ 

RM = rm -f


INC 		= 		inc/server/Client.hpp \
					inc/server/Server.hpp \
					inc/server/Channel.hpp \
					inc/commands/CommandHandler.hpp \

FILES 		= 		main.cpp \
					./src/server/Server.cpp \
					./src/server/Channel.cpp \
					./src/server/Client.cpp \
					./src/server/Boot.cpp \
					./src/commands/Command.cpp \
					./src/commands/CommandHandler.cpp \
					./src/commands/Commands.cpp \
					./src/commands/Mode.cpp \
					./src/commands/Join.cpp \
					./src/commands/Op_Commands.cpp \


OBJCS		=		$(FILES:.cpp=.o)





all:	$(NAME)

$(NAME): $(OBJCS)
	@($(CC) $(OBJCS) $(CPPFLAGS) -o $(NAME))
	@echo "✅ $(NAME) is ready !"

%.o: %.cpp $(INC)
	@($(CC) $(CPPFLAGS) -c $< -o $@)

clean:
	@echo "🧹Removing object files..."
	@($(RM) $(OBJCS))

fclean: clean
	@echo "🧹 Removing the executable $(NAME)..."
	@($(RM) $(NAME))

re: fclean all

.PHONY: clean