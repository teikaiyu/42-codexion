NAME = codexion
CFLAGS = -Wall -Wextra -Werror -pthread

SRC_DIR = srcs
INC_DIR = includes
OBJ_DIR = obj

SRCS = main.c parsing.c init.c heap.c dongle.c coder.c \
coder_phases.c monitor.c utils.coder
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

all: $(NAME)

$(NAME): $(OBJS)
	cc $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	cc $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean:
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
