# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/12/19 16:57:04 by jveirman          #+#    #+#              #
#    Updated: 2025/04/04 16:22:48 by jveirman         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS = main.cpp \
		server/WebServer.cpp \
		server/Server.cpp \
		server/Client.cpp \
		server/method.cpp \
		server/utils.cpp \
		parse/Config.cpp \
		parse/ServerConfig.cpp \
		parse/LocationConfig.cpp

NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror
STD =#-std=c++98
ifdef DEV
	DEV_FLAGS = -g3 -fsanitize=address
	# DEV_FLAGS = -Wno-shadow
else
	DEV_FLAGS =
endif

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(STD) $(DEV_FLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) $(STD) $(DEV_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
fclean : clean
	rm -f $(NAME)
re: fclean all
.PHONY: all clean fclean re
