CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pedantic -O2
LDFLAGS = 
TARGET = libkrown_auth.a
SO_TARGET = libkrown_auth.so
STATIC_LIB = $(TARGET)
SHARED_LIB = $(SO_TARGET)
OBJECTS = krown_auth.o
HEADER = krown_auth.h
EXECUTABLE = krown_auth

# Par défaut, compiler l'exécutable krown_auth
all: $(EXECUTABLE)

# Bibliothèque statique
$(STATIC_LIB): $(OBJECTS)
	ar rcs $(STATIC_LIB) $(OBJECTS)
	@echo "Bibliothèque statique $(STATIC_LIB) créée avec succès"

# Bibliothèque partagée (optionnelle)
shared: CFLAGS += -fPIC
shared: $(SHARED_LIB)

$(SHARED_LIB): $(OBJECTS)
	$(CC) -shared -o $(SHARED_LIB) $(OBJECTS) $(LDFLAGS)
	@echo "Bibliothèque partagée $(SHARED_LIB) créée avec succès"

# Compilation des objets
$(OBJECTS): krown_auth.c $(HEADER)
	$(CC) $(CFLAGS) -c krown_auth.c -o $(OBJECTS)

# Exécutable krown_auth (script principal)
$(EXECUTABLE): krown_auth_main.c krown_auth.c $(HEADER)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) krown_auth_main.c krown_auth.c $(LDFLAGS)
	@echo "Exécutable $(EXECUTABLE) créé avec succès"

# Bibliothèque statique (optionnel)
lib: $(STATIC_LIB)

# Installation (optionnel)
install: $(STATIC_LIB) $(HEADER)
	@echo "Installation de la bibliothèque..."
	@mkdir -p /usr/local/lib
	@mkdir -p /usr/local/include
	cp $(STATIC_LIB) /usr/local/lib/
	cp $(HEADER) /usr/local/include/
	@echo "Installation terminée"

# Installation de l'exécutable (optionnel)
install-bin: $(EXECUTABLE)
	@echo "Installation de l'exécutable..."
	@mkdir -p /usr/local/bin
	cp $(EXECUTABLE) /usr/local/bin/
	chmod +x /usr/local/bin/$(EXECUTABLE)
	@echo "Installation terminée. Vous pouvez maintenant utiliser: krown_auth"

# Nettoyage
clean:
	rm -f $(OBJECTS) $(STATIC_LIB) $(SHARED_LIB) $(EXECUTABLE)
	@echo "Nettoyage terminé"

.PHONY: all lib shared install install-bin clean

