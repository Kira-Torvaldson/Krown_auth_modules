# Guide Docker pour krown_auth

Ce guide explique comment utiliser Docker et Docker Compose pour compiler et tester le module krown_auth sur différentes distributions Linux.

## Prérequis

- Docker installé
- Docker Compose installé (généralement inclus avec Docker Desktop)

## Utilisation rapide

### Tester sur Debian (par défaut)

```bash
cd docker
docker-compose up krown-auth-test
```

### Tester sur différentes distributions

```bash
cd docker

# Debian
docker-compose up krown-auth-debian

# Ubuntu
docker-compose up krown-auth-ubuntu

# Arch Linux
docker-compose up krown-auth-arch
```

## Commandes utiles

### Construire toutes les images

```bash
cd docker
docker-compose build
```

### Construire une image spécifique

```bash
cd docker
docker-compose build krown-auth-debian
docker-compose build krown-auth-ubuntu
docker-compose build krown-auth-arch
```

### Exécuter en mode interactif

```bash
cd docker
docker-compose run --rm krown-auth-test /bin/bash
```

### Voir les logs

```bash
cd docker
docker-compose logs krown-auth-test
```

### Nettoyer

```bash
cd docker

# Arrêter et supprimer les conteneurs
docker-compose down

# Supprimer aussi les volumes
docker-compose down -v

# Supprimer aussi les images
docker-compose down --rmi all
```

## Volumes

Les clés SSH générées sont stockées dans des volumes Docker nommés :
- `krown-ssh-debian`
- `krown-ssh-ubuntu`
- `krown-ssh-arch`
- `krown-ssh-test`

Pour inspecter un volume :

```bash
docker volume inspect krown_auth_modules_krown-ssh-test
```

## Structure des images

Le Dockerfile utilise un build multi-stage :

1. **Stages de build** : Compilent le module sur chaque distribution
   - `debian-builder`
   - `ubuntu-builder`
   - `arch-builder`

2. **Stages runtime** : Images légères avec seulement l'exécutable et OpenSSH
   - `debian-runtime`
   - `ubuntu-runtime`
   - `arch-runtime`

## Personnalisation

Pour modifier le comportement, éditez `docker/docker-compose.yml` :

- Changer la commande par défaut
- Ajouter des variables d'environnement
- Modifier les volumes montés
- Ajouter des ports si nécessaire

## Dépannage

### Erreur de permission

Si vous avez des erreurs de permission avec les volumes, utilisez des volumes Docker nommés (déjà configurés) au lieu de volumes bind.

### Image ne se construit pas

Vérifiez que tous les fichiers sources sont présents dans la structure :
- `include/krown_auth.h`
- `src/krown_auth.c`
- `src/krown_auth_main.c`
- `Makefile`

### OpenSSH non trouvé

Les images runtime incluent OpenSSH. Si vous avez des problèmes, vérifiez que le stage de build a bien installé `openssh-client`.

