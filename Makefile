NAME = codexion
CCWP = cc -Wall -Wextra -Werror -pthread

SRC_DIR = srcs
INC_DIR = includes
OBJ_DIR = obj

SRC = $(SRC_DIR)/cleanup.c \
	$(SRC_DIR)/coder_phases.c \
	$(SRC_DIR)/coder.c \
	$(SRC_DIR)/dongle.c \
	$(SRC_DIR)/heap.c \
	$(SRC_DIR)/init.c \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/monitor.c \
	$(SRC_DIR)/parsing.c \
	$(SRC_DIR)/utils.c
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CCWP) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CCWP) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
