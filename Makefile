# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/12/19 16:57:04 by jveirman          #+#    #+#              #
#    Updated: 2025/09/16 12:13:17 by jveirman         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS = main.cpp \
		server/WebServer.cpp \
		server/Server.cpp \
		server/Client.cpp \
		server/method.cpp \
		server/utils.cpp \
		server/cookies_session.cpp \
		server/Signals.cpp \
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

all: $(NAME) purge prepareEval

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
	@if ! cp ../evaluator.conf ./config/ 2>/dev/null; then \
		echo "\e[33mevaluator.conf not found, using default configuration.\e[0m"; \
	else \
		echo "\e[32mevaluator.conf copied to config/ directory.\e[0m"; \
	fi

purge:
	@echo "Purging hack.html"
	@cp www/hack.template.html www/hack.html
	@echo "hack.html purged and restored to clean template"

.PHONY: all clean fclean re prepareEval purge
