##
## EPITECH PROJECT, 2026
## R-Type
## File description:
## Makefile
##

NAME = rtype_client

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -fexceptions
CPPFLAGS = -Iinclude -I../network/include -I../engine/include -I../engine/include/components

LDFLAGS = -lsfml-graphics	\
          -lsfml-window	\
          -lsfml-system	\
          -lcrypto

SRC = src/main.cpp	\
      src/App.cpp	\
      src/AssetsPak.cpp	\
      src/Animation.cpp	\
      src/Resources.cpp	\
      src/ClientNet.cpp	\
      src/ecs/Systems.cpp	\
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