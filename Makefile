# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rpisoner <rpisoner@student.42madrid.com>   +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/04/29 12:35:06 by jolivare          #+#    #+#              #
#    Updated: 2025/06/24 13:19:13 by rpisoner         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

SRC = parseConfig/Server.cpp parseConfig/Location.cpp main.cpp parseConfig/HttpRequest.cpp \
      parseConfig/HttpResponse.cpp parseConfig/HttpHandler.cpp parseConfig/Config.cpp \
	  parseConfig/SessionManager.cpp
OBJECTS = $(SRC:.cpp=.o)
CC = c++
RM = rm -f
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -g3

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CPPFLAGS) $(OBJECTS) -o $(NAME)

clean:
	$(RM) $(OBJECTS)

fclean: clean
	$(RM) $(NAME)

c: all clean

re: fclean all

run: all clean
	./$(NAME) config.conf

.PHONY: all clean fclean re run c