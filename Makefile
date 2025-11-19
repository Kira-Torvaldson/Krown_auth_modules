# Makefile pour krown_auth
# Structure organisée avec src/, include/, build/

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pedantic -O2 -Iinclude -D_FORTIFY_SOURCE=2 -fstack-protector-strong
LDFLAGS = -Wl,-z,relro,-z,now 
TARGET = libkrown_auth.a
SO_TARGET = libkrown_auth.so
STATIC_LIB = build/$(TARGET)
SHARED_LIB = build/$(SO_TARGET)
OBJECTS = build/krown_auth.o
HEADER = include/krown_auth.h
EXECUTABLE = build/krown_auth

# Sources
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
SOURCES = $(SRC_DIR)/krown_auth.c
MAIN_SOURCE = $(SRC_DIR)/krown_auth_main.c

# Créer le dossier build s'il n'existe pas
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Par défaut, compiler l'exécutable krown_auth
all: $(BUILD_DIR) $(EXECUTABLE)

# Bibliothèque statique
$(STATIC_LIB): $(BUILD_DIR) $(OBJECTS)
	ar rcs $(STATIC_LIB) $(OBJECTS)
	@echo "✓ Bibliothèque statique $(STATIC_LIB) créée avec succès"

# Bibliothèque partagée (optionnelle)
shared: CFLAGS += -fPIC
shared: $(BUILD_DIR) $(SHARED_LIB)

$(SHARED_LIB): $(OBJECTS)
	$(CC) -shared -o $(SHARED_LIB) $(OBJECTS) $(LDFLAGS)
	@echo "✓ Bibliothèque partagée $(SHARED_LIB) créée avec succès"

# Compilation des objets
$(OBJECTS): $(SOURCES) $(HEADER) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(SOURCES) -o $(OBJECTS)

# Exécutable krown_auth (script principal)
$(EXECUTABLE): $(MAIN_SOURCE) $(SOURCES) $(HEADER) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(MAIN_SOURCE) $(SOURCES) $(LDFLAGS)
	@echo "✓ Exécutable $(EXECUTABLE) créé avec succès"

# Bibliothèque statique (optionnel)
lib: $(STATIC_LIB)

# Installation (optionnel)
install: $(STATIC_LIB) $(HEADER)
	@echo "Installation de la bibliothèque..."
	@mkdir -p /usr/local/lib
	@mkdir -p /usr/local/include
	cp $(STATIC_LIB) /usr/local/lib/
	cp $(HEADER) /usr/local/include/
	@echo "✓ Installation terminée"

# Installation de l'exécutable (optionnel)
install-bin: $(EXECUTABLE)
	@echo "Installation de l'exécutable..."
	@mkdir -p /usr/local/bin
	cp $(EXECUTABLE) /usr/local/bin/
	chmod +x /usr/local/bin/krown_auth
	@echo "✓ Installation terminée. Vous pouvez maintenant utiliser: krown_auth"

# Nettoyage
clean:
	rm -rf $(BUILD_DIR)
	@echo "✓ Nettoyage terminé"

# Aide
help:
	@echo "Makefile pour krown_auth"
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  make          - Compile l'exécutable krown_auth (défaut)"
	@echo "  make lib      - Compile la bibliothèque statique"
	@echo "  make shared   - Compile la bibliothèque partagée"
	@echo "  make install  - Installe la bibliothèque"
	@echo "  make install-bin - Installe l'exécutable"
	@echo "  make clean    - Nettoie les fichiers de compilation"
	@echo "  make help     - Affiche cette aide"

.PHONY: all lib shared install install-bin clean help
