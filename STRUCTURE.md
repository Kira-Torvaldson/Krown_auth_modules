# Structure du projet krown_auth

## Organisation des fichiers

```
Krown_auth_modules/
├── src/                      # Code source
│   ├── krown_auth.c          # Implémentation du module
│   └── krown_auth_main.c     # Point d'entrée du script krown_auth
│
├── include/                  # En-têtes
│   └── krown_auth.h          # En-tête du module (API publique)
│
├── build/                    # Fichiers de compilation (généré, ignoré par Git)
│   ├── krown_auth            # Exécutable
│   ├── krown_auth.o          # Objet compilé
│   ├── libkrown_auth.a       # Bibliothèque statique
│   └── libkrown_auth.so      # Bibliothèque partagée
│
├── docs/                     # Documentation
│   └── DOCKER.md             # Guide Docker
│
├── docker/                   # Configuration Docker
│   ├── Dockerfile            # Dockerfile multi-stage
│   └── docker-compose.yml    # Docker Compose
│
├── Makefile                  # Fichier de compilation
├── README.md                 # Documentation principale
├── STRUCTURE.md              # Ce fichier (description de la structure)
└── .gitignore                # Fichiers à ignorer par Git
```

## Description des dossiers

### `src/`
Contient tous les fichiers sources C (`.c`).
- `krown_auth.c` : Implémentation complète du module
- `krown_auth_main.c` : Point d'entrée pour l'exécutable `krown_auth`

### `include/`
Contient tous les fichiers d'en-tête (`.h`).
- `krown_auth.h` : API publique du module

### `build/`
Dossier généré automatiquement lors de la compilation.
- Contient tous les fichiers compilés (objets, bibliothèques, exécutables)
- Ignoré par Git (dans `.gitignore`)

### `docs/`
Documentation supplémentaire.
- `DOCKER.md` : Guide d'utilisation Docker

### `docker/`
Configuration Docker pour tester sur différentes distributions.
- `Dockerfile` : Build multi-stage pour Debian, Ubuntu, Arch
- `docker-compose.yml` : Services Docker Compose

## Avantages de cette structure

1. **Séparation claire** : Sources, en-têtes et build sont séparés
2. **Maintenabilité** : Facile de trouver et modifier les fichiers
3. **Standards** : Structure conforme aux pratiques C courantes
4. **Scalabilité** : Facile d'ajouter de nouveaux fichiers
5. **Propreté** : Les fichiers générés sont dans `build/`, pas à la racine

## Compilation

Le Makefile est configuré pour cette structure :

```bash
make              # Compile dans build/krown_auth
make lib          # Compile dans build/libkrown_auth.a
make clean        # Supprime le dossier build/
```

