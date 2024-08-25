NAME		=	webserv
CC			=	c++
CFLAGS		=	-Wall -Wextra -Werror -fsanitize=address -std=c++11
# CFLAGS		=	-Wall -Wextra -Werror -std=c++98 -fsanitize=address

INC_DIR		=	inc
INC_FILES	=	webserv.hpp client.hpp server.hpp request.hpp config.hpp response.hpp \
				types.hpp autoindex.hpp cgi.hpp error.hpp
INC			=	$(addprefix $(INC_DIR)/, $(INC_FILES))

SRCS_DIR	=	srcs
SRCS_FILES	=	main.cpp webserv.cpp server.cpp client.cpp request.cpp config.cpp response.cpp \
				types.cpp autoindex.cpp cgi.cpp error.cpp
SRCS		=	$(addprefix $(SRCS_DIR)/, $(SRCS_FILES))

OBJS_DIR	=	objs
OBJS		=	$(addprefix $(OBJS_DIR)/, $(SRCS_FILES:.cpp=.o))


all	:	$(NAME)

$(NAME)	:	$(OBJS)
	@$(CC) $(CFLAGS) $^ -o $@

$(OBJS_DIR)/%.o	:	$(SRCS_DIR)/%.cpp $(INC)
	@mkdir -p $(OBJS_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

clean	:
	@rm -rf $(OBJS_DIR)

fclean	:	clean
	@rm -rf $(NAME)

re	:	fclean all

.PHONY	:	all clean fclean re