# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rpisoner <rpisoner@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/04/29 12:35:06 by jolivare          #+#    #+#              #
#    Updated: 2025/06/24 18:40:42 by rpisoner         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

#########################################################################################
# Sources & objects
#########################################################################################

SRC = parseConfig/Server.cpp parseConfig/Location.cpp main.cpp parseConfig/HttpRequest.cpp \
      parseConfig/HttpResponse.cpp parseConfig/HttpHandler.cpp parseConfig/Config.cpp \
	  parseConfig/SessionManager.cpp
OBJECTS = $(SRC:.cpp=.o)
CC = c++
RM = rm -f
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -g3

#########################################################################################
# Colors
#########################################################################################

DEF_COLOR = \033[0;39m
CUT = \033[K
R = \033[31;1m
G = \033[32;1m
Y = \033[33;1m
B = \033[34;1m
P = \033[35;1m
GR = \033[30;1m
END = \033[0m

#########################################################################################
# Compilation rules
#########################################################################################

all: $(NAME)

$(NAME): $(OBJECTS)
	@$(CC) $(CPPFLAGS) $(OBJECTS) -o $(NAME)
	@printf "$(CUT)\r$(G)Webserv compiled!💻$(END)\n"

%.o: %.cpp
	@printf "$(Y)Compiling: $<...$(CUT)\r$(END)"
	@$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	@$(RM) $(OBJECTS)
	@printf "$(R)All .o files removed$(END)\n"

fclean: clean
	@$(RM) $(NAME)
	@printf "$(R)Executable file removed$(END)\n"

c: all clean

re: fclean all

run: all clean
	./$(NAME) config.conf

.PHONY: all clean fclean re run c