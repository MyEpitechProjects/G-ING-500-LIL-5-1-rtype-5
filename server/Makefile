##
## EPITECH PROJECT, 2026
## Raytracer
## File description:
## Makefile
##

NAME =      rtype_server

CXX =       g++
CXXFLAGS =  -std=c++20 -Wall -Wextra -Werror -fexceptions
CPPFLAGS =  -Iinclude \
			-I../network/include

SRC = 	src/main.cpp \
		src/Server.cpp \
		../network/src/WakeSignal.cpp \
		../network/src/PacketCodec.cpp \
		../network/src/NetworkHandler.cpp \
		../network/src/Paquet.cpp

OBJ = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJ)
		$(CXX) $(OBJ) -o $@ $(LDFLAGS)

clean:
		rm -rf $(OBJ)

fclean: clean
		rm -f $(NAME)

re: fclean all


.PHONY: all clean fclean re plugins docs
