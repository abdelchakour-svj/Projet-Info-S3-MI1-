
# Makefile - Compilation du projet C-Wildwater

# Ce fichier permet de compiler le programme C du projet.
# Usage:
#   make        - Compile l'executable
#   make clean  - Supprime les fichiers generes


CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = wildwater

# Fichiers sources et objets
SRCS = main.c avl.c arbre_distrib.c
OBJS = $(SRCS:.c=.o)

# Regle principale (premiere cible)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compilation des fichiers objets
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependances des headers
main.o: main.c avl.h arbre_distrib.h
avl.o: avl.c avl.h
arbre_distrib.o: arbre_distrib.c arbre_distrib.h

# Nettoyage
clean:
	rm -f $(OBJS) $(TARGET)

# Recompilation complete
rebuild: clean $(TARGET)

.PHONY: clean rebuild
