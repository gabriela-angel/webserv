#pragma once

#include <string>

# define COLORIZE(color, text) (color + std::string(text) + "\033[0m")

# define RED "\033[31m"
# define GRAY "\033[90m"
# define BLUE "\033[34m"
# define CYAN "\033[36m"
# define BLACK "\033[30m"
# define GREEN "\033[32m"
# define WHITE "\033[37m"
# define YELLOW "\033[33m"
# define MAGENTA "\033[35m"
# define RESET "\033[0m"
# define BRIGHT_RED "\033[91m"
# define BRIGHT_BLUE "\033[94m"
# define BRIGHT_GREEN "\033[92m"
# define BRIGHT_YELLOW "\033[93m"