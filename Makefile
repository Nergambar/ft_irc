NAME = ircserv

CXX = c++
CXXFLAGS =  -Wall -Wextra -Werror -std=c++98 -Iincludes -g

SRC 	= main.cpp users.cpp operators.cpp servers.cpp commands.cpp recvLoop.cpp handleClient.cpp handleCommands.cpp server_helpers/helper1.cpp channel.cpp parsing/parsing1.cpp
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