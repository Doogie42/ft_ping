SRC=main.c parse_arg.c printer.c setup.c packet.c

OBJSRC=$(SRC:.c=.o)
OBJDIR=obj

OBJ=$(addprefix $(OBJDIR)/, $(OBJSRC))
NAME=ft_ping
FLAG=-Wall -Wextra -Werror 

all: $(NAME)

$(NAME): $(OBJ)
	cc $(FLAG) -lm $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: %.c
	mkdir -p obj
	cc $(FLAG) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

run: $(NAME)
	@echo "\n"
	@./$(NAME)
.PHONY: all clean fclean re