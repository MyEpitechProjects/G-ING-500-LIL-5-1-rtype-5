##
## EPITECH PROJECT, 2026
## R-Type
## File description:
## Makefile
##

NAME = rtype_client

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -fexceptions
CPPFLAGS = -Iinclude -I../network/include

LDFLAGS = -lsfml-graphics	\
          -lsfml-window	\
          -lsfml-system	\
          -lcrypto

SRC = src/main.cpp	\
      src/Graphic.cpp	\
      src/AssetsPak.cpp	\
      src/objects/Object.cpp	\
      src/Animation.cpp	\
      src/objects/PlayerObject.cpp	\
      src/ObjectManager.cpp	\
      src/objects/ShootLoadingObject.cpp	\
      src/objects/NetworkObject.cpp	\
      src/objects/EnemyObject.cpp	\
      ../network/src/WakeSignal.cpp	\
      ../network/src/PacketCodec.cpp	\
      ../network/src/NetworkHandler.cpp	\
      ../network/src/Paquet.cpp

OBJ = $(SRC:.cpp=.o)

PYTHON = python3

ASSETS_DIR = assets
ASSETS_ARCHIVE = assets.pak
ASSET_PACKER = tools/pack_assets.py

ASSET_FILES = $(shell find $(ASSETS_DIR) -type f)

.PHONY: all clean fclean re

all: $(NAME)

$(ASSETS_ARCHIVE): $(ASSET_FILES) $(ASSET_PACKER)
	$(PYTHON) $(ASSET_PACKER) $(ASSETS_DIR) $(ASSETS_ARCHIVE)

$(NAME): $(ASSETS_ARCHIVE) $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

src/AssetsPak.o: $(ASSETS_ARCHIVE)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME) $(ASSETS_ARCHIVE)

re: fclean all