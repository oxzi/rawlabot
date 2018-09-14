CC     ?= gcc
CFLAGS  = -Wall -Wextra -pedantic -Ofast -lWalabotAPI -D__LINUX__
SRC     = rawlabot.cpp
OBJ     = $(SRC:.cpp=.o)
OUTPUT  = rawlabot

default:
	$(CC) $(CFLAGS) -c $(SRC)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJ)

clean:
	$(RM) $(OUTPUT) $(OBJ)
