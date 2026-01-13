#MAKEFLAGS += --silent

CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -g3
CDEBUGFLAGS = -g
INC_DIR = inc/
INC = ft_traceroute.h

EXE = ft_traceroute

SRC_DIR = src/
SRC =	main.c \
		probe.c \
		output.c \
		ft_strcmp.c \
		ft_memset.c

OBJ_DIR = .obj/
OBJ = $(SRC:%.c=$(OBJ_DIR)%.o)

.PHONY : all clean fclean re $(EXE)

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXE)
	@echo "------------\n"

$(OBJ_DIR)%.o: $(SRC_DIR)%.c $(addprefix $(INC_DIR),$(INC))
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_DIR)

run: all
	sudo ./$(EXE) 42.fr

strace: $(EXE)
	strace ./$(EXE)

container:
	docker build . -t ft_traceroute
	docker run -d --rm -v ./:/shared --name ft_traceroute ft_traceroute

shell:
	docker exec -it ft_traceroute /bin/bash

rm_container:
	docker stop ft_traceroute
	docker container prune

clean:
	@/bin/rm -rf $(OBJ_DIR)

fclean: clean
	@/bin/rm -f $(EXE)

re:
	$(MAKE) fclean
	$(MAKE) all
