# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jolivare <jolivare@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/04/29 12:35:06 by jolivare          #+#    #+#              #
#    Updated: 2025/06/15 16:42:44 by jolivare         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

SRC = parseConfig/Server.cpp parseConfig/Location.cpp main.cpp parseConfig/HttpRequest.cpp \
      parseConfig/HttpResponse.cpp parseConfig/HttpHandler.cpp parseConfig/Config.cpp
OBJECTS = $(SRC:.cpp=.o)
CC = c++
RM = rm -f
CPPFLAGS = -Wall -Wextra -Werror -std=c++98

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