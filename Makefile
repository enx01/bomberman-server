CC = gcc
CFLAGS = -Wall
LDFLAGS = -lncurses
BIN_DIR = bin
SRC_DIR = src
CURR_DIR = $(shell pwd)

# Defintion du nom de l'executable
SERVER_TARGET = $(BIN_DIR)/server

# Cible par defaut
all: $(SERVER_TARGET)

SERVER_SRCS :=  $(shell find $(SRC_DIR) -name '*.c')

$(SERVER_TARGET):
	mkdir -p $(BIN_DIR)
	$(CC) -o $(SERVER_TARGET) $(SERVER_SRCS) $(LDFLAGS)

# Nettoyage des artefacts de compilation.
clean:
	rm -rf $(BIN_DIR)

rserver :
	xterm -bg black -fg white -e $(CURR_DIR)/$(BIN_DIR)/server &


# Phony targets (not associated with files)
.PHONY: all clean


