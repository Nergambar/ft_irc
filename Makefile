NAME = ircserv

CXX = c++
CXXFLAGS =  -Wall -Wextra -Werror -std=c++98 -Iincludes -g

SRC 	= server/servers.cpp server/helper1.cpp server/handleClient.cpp \
			parsing/parsing1.cpp \
			commands.cpp passUser.cpp  handleCommands.cpp  channel.cpp \
			main.cpp users.cpp operators.cpp
OBJ 	= $(SRC:.cpp=.o)


all: $(NAME)
	@echo compiling...

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	@echo cleaning $(OBJ)...

fclean: clean
	rm -f $(NAME)
	@echo removing $(NAME)

re: fclean all

.PHONY: all clean fclean re

.SILENT: