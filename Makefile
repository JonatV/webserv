CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = webserv

SRCS = $(shell find . -name "*.cpp" -type f | sed 's|./||')
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

