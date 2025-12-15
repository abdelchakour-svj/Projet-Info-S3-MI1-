# Makefile pour le projet C-Wildwater
# Compile le programme de traitement des donnees d'eau

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -g

# Nom de l'executable
TARGET = wildwater

# Fichiers sources et objets
SRCS = main.c avl.c arbre_distrib.c
OBJS = $(SRCS:.c=.o)

# Regle par defaut: compiler l'executable
all: $(TARGET)

# Lier les fichiers objets pour creer l'executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compiler les fichiers .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependances
main.o: main.c avl.h arbre_distrib.h
avl.o: avl.c avl.h
arbre_distrib.o: arbre_distrib.c arbre_distrib.h

# Nettoyer les fichiers generes
clean:
	rm -f $(OBJS) $(TARGET)

# Nettoyer tout (y compris les fichiers de donnees)
mrproper: clean
	rm -f *.dat *.png

# Recompiler tout
rebuild: clean all

.PHONY: all clean mrproper rebuild
