##
## EPITECH PROJECT, 2026
## Raytracer
## File description:
## Makefile
##

NAME =      rtype_client

CXX =       g++
CXXFLAGS =  -std=c++20 -Wall -Wextra -Werror -fexceptions

CPPFLAGS =  -Iinclude \
			-I../network/include

LDFLAGS =   -lsfml-graphics \
			-lsfml-window \
			-lsfml-system \
			-lcrypto

SRC =       src/main.cpp \
			src/Graphic.cpp \
			src/AssetsPak.cpp	\
			src/objects/Object.cpp	\
			src/Animation.cpp	\
			src/objects/PlayerObject.cpp	\
			src/ObjectManager.cpp	\
			src/objects/ShootLoadingObject.cpp	\
			src/objects/NetworkObject.cpp	\
			../network/src/WakeSignal.cpp \
			../network/src/PacketCodec.cpp \
			../network/src/NetworkHandler.cpp \
			../network/src/Paquet.cpp \

OBJ = $(SRC:%.cpp=%.o)

PYTHON = python3

ASSETS_DIR = assets
ASSETS_ARCHIVE = assets.pak
ASSET_PACKER = tools/pack_assets.py

ASSET_FILES = $(shell find $(ASSETS_DIR) -type f)

$(ASSETS_ARCHIVE): $(ASSET_FILES) $(ASSET_PACKER)
	$(PYTHON) $(ASSET_PACKER) $(ASSETS_DIR) $(ASSETS_ARCHIVE)

all: $(ASSETS_ARCHIVE) $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -rf $(OBJ)
	rm -rf $(ASSETS_ARCHIVE)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re