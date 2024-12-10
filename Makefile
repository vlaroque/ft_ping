# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vlaroque <vlaroque@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/06/20 10:33:13 by vlaroque          #+#    #+#              #
#    Updated: 2019/10/10 06:50:48 by vlaroque         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# GLOBAL

NAME = ft_ping
CC = gcc
CFLAGS = -Wextra -Werror -Wall

# SOURCES

SRC_FILES = ft_ping.c init.c icmp_packet.c dest.c

# PATH

SRC_PATH = src
OBJ_PATH = obj
INC_PATH = inc

# ASSIGNATION

SRC_FILES_FULL_PATH = $(addprefix $(SRC_PATH)/,$(SRC_FILES))
OBJ_FILES = $(SRC_FILES:.c=.o)
OBJ_FILES_FULL_PATH = $(addprefix $(OBJ_PATH)/,$(OBJ_FILES))
DEPENDS = $(OBJ_FILES_FULL_PATH:.o=.d)

.PHONY: all
all: $(NAME)

$(NAME): $(OBJ_FILES_FULL_PATH)
	@echo "\tLinking $@'s files"
	@$(CC) $(OBJ_FILES_FULL_PATH) -o $@ $(CFLAGS)
	@echo "\t\tDone !"

-include $(DEPENDS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	@mkdir -p $(@D)
	@echo "\tCompiling $@"
	@$(CC) $(CFLAGS) -I$(INC_PATH) -MMD -c $< -o $@

.PHONY: clean
clean:
	@echo "\tCleaning..."
	@rm -f $(OBJ_FILES_FULL_PATH)
	@rm -f $(DEPENDS)
	@echo "\t\tDone !"

.PHONY: fclean
fclean: clean
	@rm -f $(NAME)

.PHONY: re
re:
	@$(MAKE) fclean
	@$(MAKE)
