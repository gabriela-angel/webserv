# ===========================================================================
#  =============================== CONTROL =================================
# ===========================================================================

UP = \033[A
CUT = \033[K
RESET = \033[0m
RED = \033[31;3m
GREEN = \033[32;3m
YELLOW = \033[33;3m
WHITE = \033[37;1;4m
CYAN = \033[36;1;3;208m

# ===========================================================================
#  =============================== COMPILER ================================
# ===========================================================================

CFLAGS := -Wall -Wextra -Werror -std=c++98 -g3
VALGRIND_LOG := valgrind.log
CC := c++

# ===========================================================================
#  ================================ WEBSERV ================================
# ===========================================================================

NAME = webserv
SRC_PATH = ./src/
HEADER_PATH = ./include/
BUILD_PATH = ./build/
LOGS_PATH = ./logs/

UTILS_PATH = ./utils/
FILES = \
	main.cpp \
	$(UTILS_PATH)Logger.cpp \

OBJS = $(addprefix $(BUILD_PATH), $(FILES:%.cpp=%.o))

# ===========================================================================
#  ================================= RULES =================================
# ===========================================================================

all: $(BUILD_PATH) print $(NAME)

$(BUILD_PATH):
	@mkdir -p $(addprefix $(BUILD_PATH), $(dir $(FILES)))
	@mkdir -p $(LOGS_PATH)

print:
ifeq ($(wildcard $(NAME)),)
	@printf "$(GREEN) ------------------------$(RESET)"
	@printf "$(GREEN)| Compiling Main Project |$(RESET)"
	@printf "$(GREEN)------------------------$(RESET)"
	@echo " "
endif

$(NAME): $(OBJS)
	@printf "$(YELLOW)[Building]$(RESET) $(NAME)...\n"
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS) -I$(HEADER_PATH) -L$(LIB_PATH) -lft $(RFLAGS)
	@printf "$(UP)$(CUT)"
	@printf "$(GREEN)[Builded]$(RESET) $(NAME)...\n"
	@printf "$(CYAN)------ ----------------------------------------------- ------$(RESET)\n"
	@printf "$(CYAN)------| WEBSERV executable was created successfully!! |------$(RESET)\n"
	@printf "$(CYAN)------ ----------------------------------------------- ------$(RESET)\n"
	@echo " "

$(BUILD_PATH)%.o: $(SRC_PATH)%.cpp
	@printf "$(YELLOW)[Compiling]$(RESET) $(notdir $<)...\n"
	@$(CC) $(CFLAGS) -c $< -o $@ -I$(HEADER_PATH)
	@printf "$(UP)$(CUT)"
	@printf "$(GREEN)[Compiled]$(RESET) $(notdir $<)...\n"


clean:
	@printf "$(RED)[Removing Objects...]$(RESET)\n"
	@rm -rf $(BUILD_PATH)
	@rm -rf $(VALGRIND_LOG)

fclean: clean
	@printf "$(RED)[Removing $(notdir $(NAME))...]$(RESET)\n"
	@rm -rf $(LOGS_PATH)
	@rm -rf $(NAME)

re: fclean
	@make --no-print-directory

valgrind: all
	@valgrind --leak-check=full \
	--show-reachable=yes \
	--track-fds=yes \
	--show-leak-kinds=all -s \
	--track-origins=yes \
	--log-file=$(VALGRIND_LOG) \
	./$(NAME)
	@cat $(VALGRIND_LOG)

.PHONY: all clean fclean re valgrind
