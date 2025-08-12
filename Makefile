# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/12/19 16:57:04 by jveirman          #+#    #+#              #
#    Updated: 2025/08/12 16:24:28 by jveirman         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS = main.cpp \
		server/WebServer.cpp \
		server/Server.cpp \
		server/Client.cpp \
		server/method.cpp \
		server/utils.cpp \
		server/cookies_session.cpp \
		parse/Config.cpp \
		parse/ServerConfig.cpp \
		parse/LocationConfig.cpp \
		misc/Evaluator.cpp

NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror
STD = -std=c++98
ifdef DEV
	DEV_FLAGS = -g3 -fsanitize=address
	# DEV_FLAGS = -Wno-shadow
else
	DEV_FLAGS =
endif

OBJS = $(SRCS:.cpp=.o)

all: $(NAME) prepareEval

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(STD) $(DEV_FLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) $(STD) $(DEV_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
fclean : clean
	rm -f $(NAME)
re: fclean all

prepareEval:
	cp ../evaluator.conf ./config/ || true

purge:
	@echo "Purging hack.html"
	@cp www/hack.template.html www/hack.html
	@echo "hack.html purged and restored to clean template"

.PHONY: all clean fclean re prepareEval purge
