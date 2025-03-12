CXX = c++
CXXFLAGS = -std=c++98 -Werror -Wextra -Wall #-Wpedantic
NAME = webserv
FILES = main.cpp
SOCKETS = src/networking/sockets/Socket.cpp \
		src/networking/sockets/ConnectingSocket.cpp \
		src/networking/sockets/ListeningSocket.cpp \
		src/networking/sockets/BindingSocket.cpp \
		src//server/PollHandler.cpp

SERVER = src/server/SimpleServer.cpp \
		src/server/Server.cpp \

OBJS = $(FILES:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
		$(CXX) $(CXXFLAGS) $(SERVER) $(SOCKETS) $(FILES) -o $(NAME)

src/%.o: %.cpp
		$(CXX) $(CXXFLAGS) -c $<

clean:
		rm -f $(OBJS)

re: clean all

fclean : clean 
		rm -f $(NAME)

.PHONY: all clean%