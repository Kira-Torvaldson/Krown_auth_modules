# Module krown_auth

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C Standard](https://img.shields.io/badge/C-C11-green.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))

Module C pour gérer l'authentification et la préparation d'une machine virtuelle pour être utilisée avec Krown. Ce module automatise la génération et la gestion des clés SSH nécessaires à l'authentification.

Le script `krown_auth` prépare automatiquement la VM et génère les clés SSH dans le bon répertoire (`~/.ssh/`).

## 📋 Table des matières

- [Fonctionnalités](#-fonctionnalités)
- [Prérequis](#-prérequis)
- [Installation](#-installation)
- [Structure du projet](#-structure-du-projet)
- [Utilisation](#-utilisation)
- [API de référence](#-api-de-référence)
- [Exemples](#-exemples)
- [Compatibilité](#-compatibilité)
- [Dépannage](#-dépannage)
- [Contribution](#-contribution)

## ✨ Fonctionnalités

- 🚀 **Script krown_auth** : Exécutable qui prépare automatiquement la VM et génère les clés SSH
- 🚀 **Préparation automatique complète** : Une seule fonction `prepare_vm_for_krown()` fait tout automatiquement
- 🔐 **Génération automatique de clés SSH** : ED25519 (prioritaire) ou RSA 4096 (fallback)
- 📁 **Gestion automatique du dossier `.ssh`** : Création automatique avec permissions correctes (700)
- 🔒 **Correction automatique des permissions** : Vérification et correction automatique des permissions de sécurité
  - Clé privée : `600` (-rw-------)
  - Clé publique : `644` (-rw-r--r--)
- ✅ **Vérification d'intégrité** : Détection et validation des clés existantes
- 🔄 **Régénération automatique** : Régénération automatique si les clés sont corrompues ou illisibles
- 🔍 **Vérification OpenSSH** : Contrôle automatique de la présence du client OpenSSH
- 🎯 **Zéro configuration** : Aucune intervention manuelle nécessaire, tout est automatique

## 📦 Prérequis

- **OpenSSH client** : `ssh-keygen` doit être disponible dans le PATH
- **Compilateur C** : Compatible C11 (GCC, Clang, ou MSVC)
- **Système d'exploitation** : Linux, macOS, ou Windows (avec MinGW/MSYS ou WSL)

### Vérifier OpenSSH

```bash
ssh-keygen -V
```

Si la commande échoue, installez OpenSSH :

- **Ubuntu/Debian** : `sudo apt-get install openssh-client`
- **CentOS/RHEL** : `sudo yum install openssh-clients`
- **macOS** : Inclus par défaut
- **Windows** : Installer via [OpenSSH pour Windows](https://github.com/PowerShell/Win32-OpenSSH) ou utiliser WSL

## 🚀 Installation

### Compilation du script krown_auth

Par défaut, la compilation crée l'exécutable `krown_auth` :

```bash
make
```

Cela génère l'exécutable `krown_auth` qui prépare automatiquement la VM et crée les clés SSH.

### Installation du script (optionnel)

Pour installer le script dans `/usr/local/bin` :

```bash
sudo make install-bin
```

Après installation, vous pouvez utiliser `krown_auth` depuis n'importe où.

### Compilation de la bibliothèque (optionnel)

Si vous voulez seulement la bibliothèque pour l'intégrer dans votre projet :

```bash
# Bibliothèque statique
make lib

# Bibliothèque partagée
make shared
```

### Nettoyage

```bash
make clean
```

## 🔑 Création des clés SSH

### Utilisation du script krown_auth (recommandé)

Le moyen le plus simple de préparer la VM et créer les clés SSH :

```bash
# Compiler le script
make

# Exécuter le script
./krown_auth
```

Le script `krown_auth` va automatiquement :
- ✅ Vérifier qu'OpenSSH est installé
- ✅ Créer le dossier `~/.ssh` si nécessaire
- ✅ **Générer les clés SSH** (ED25519 ou RSA 4096) dans `~/.ssh/`
- ✅ Configurer les permissions correctement (600 pour privée, 644 pour publique)
- ✅ Vérifier l'intégrité des clés
- ✅ Afficher le chemin et le contenu de la clé publique

**Exemple de sortie :**
```
=== Krown Auth - Préparation de la VM pour Krown ===

✓ VM préparée avec succès !

Chemin de la clé publique: /home/user/.ssh/id_ed25519.pub

Clé publique (à utiliser avec l'API Krown):
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAI...

✓ La VM est maintenant prête pour Krown !
✓ Les clés SSH ont été générées et configurées correctement.
```

### Utilisation dans votre code

Pour créer les clés SSH et préparer la VM pour Krown, intégrez le module dans votre application :

```c
#include "krown_auth.h"
#include <stdio.h>

int main(void) {
    char public_key_path[512];
    
    // Cette fonction crée automatiquement les clés si elles n'existent pas
    if (prepare_vm_for_krown(public_key_path, sizeof(public_key_path)) 
        == KROWN_AUTH_SUCCESS) {
        printf("Clés créées/préparées : %s\n", public_key_path);
        return 0;
    }
    
    printf("Erreur lors de la création des clés\n");
    return 1;
}
```

**Compilation et exécution :**

```bash
# Compiler
gcc -o mon_app mon_app.c krown_auth.c -std=c11

# Exécuter (crée les clés automatiquement)
./mon_app
```

Cette fonction va automatiquement :
- ✅ Vérifier qu'OpenSSH est installé
- ✅ Créer le dossier `~/.ssh` si nécessaire
- ✅ **Créer les clés SSH** (ED25519 ou RSA 4096) si elles n'existent pas
- ✅ Configurer les permissions correctement
- ✅ Vérifier l'intégrité des clés

### Création manuelle (avancé)

Si vous voulez créer les clés manuellement sans utiliser `prepare_vm_for_krown()` :

```c
#include "krown_auth.h"

// Vérifier si les clés existent
if (!krown_keys_exist(KROWN_KEY_ED25519)) {
    // Créer les clés ED25519
    krown_generate_ssh_keys(KROWN_KEY_ED25519, false);
}
```

### Vérification manuelle

Après création, vous pouvez vérifier que les clés ont bien été créées :

```bash
# Lister les clés SSH
ls -la ~/.ssh/

# Afficher la clé publique
cat ~/.ssh/id_ed25519.pub
# ou
cat ~/.ssh/id_rsa.pub
```

### Emplacement des clés créées

Les clés sont automatiquement créées dans :
- **Clé privée ED25519** : `~/.ssh/id_ed25519`
- **Clé publique ED25519** : `~/.ssh/id_ed25519.pub`
- **Clé privée RSA** : `~/.ssh/id_rsa` (si ED25519 n'est pas disponible)
- **Clé publique RSA** : `~/.ssh/id_rsa.pub` (si ED25519 n'est pas disponible)

## 📂 Structure du projet

```
Krown_auth_modules/
├── krown_auth.h          # En-tête du module (API publique)
├── krown_auth.c          # Implémentation du module
├── krown_auth_main.c     # Point d'entrée du script krown_auth
├── Makefile              # Fichier de compilation
├── README.md             # Documentation (ce fichier)
└── .gitignore            # Fichiers à ignorer par Git
```

## 💻 Utilisation

### Intégration dans votre projet

1. **Copier les fichiers** : `krown_auth.h` et `krown_auth.c` dans votre projet
2. **Compiler** : Inclure `krown_auth.c` dans votre compilation ou lier `libkrown_auth.a`
3. **Inclure l'en-tête** : `#include "krown_auth.h"`

### Exemple minimal

```c
#include "krown_auth.h"
#include <stdio.h>

int main(void) {
    char public_key_path[512];
    
    // Une seule fonction prépare tout automatiquement !
    if (prepare_vm_for_krown(public_key_path, sizeof(public_key_path)) 
        == KROWN_AUTH_SUCCESS) {
        printf("✓ VM prête pour Krown !\n");
        printf("Clé publique: %s\n", public_key_path);
        return 0;
    }
    
    printf("✗ Erreur lors de la préparation\n");
    return 1;
}
```

**C'est tout !** La fonction `prepare_vm_for_krown()` fait automatiquement :
- ✅ Vérifie OpenSSH
- ✅ Crée/configure `~/.ssh`
- ✅ Génère les clés SSH si nécessaire
- ✅ Corrige les permissions
- ✅ Vérifie l'intégrité
- ✅ Prépare tout pour Krown

### Compilation avec votre projet

```bash
gcc -o mon_projet mon_projet.c krown_auth.c -std=c11
```

Ou avec la bibliothèque statique :

```bash
gcc -o mon_projet mon_projet.c -L. -lkrown_auth -std=c11
```

## 📚 API de référence

### Types

#### `krown_auth_result_t`

Code de retour des fonctions du module.

```c
typedef enum {
    KROWN_AUTH_SUCCESS = 0,
    KROWN_AUTH_ERROR_SSH_DIR = -1,
    KROWN_AUTH_ERROR_KEY_GEN = -2,
    KROWN_AUTH_ERROR_PERMISSIONS = -3,
    KROWN_AUTH_ERROR_OPENSSH_NOT_FOUND = -4,
    KROWN_AUTH_ERROR_READ_KEY = -5,
    KROWN_AUTH_ERROR_MEMORY = -6
} krown_auth_result_t;
```

#### `krown_key_type_t`

Type de clé SSH à utiliser.

```c
typedef enum {
    KROWN_KEY_ED25519 = 0,    // Clé ED25519 (recommandée)
    KROWN_KEY_RSA_4096 = 1    // Clé RSA 4096 bits (fallback)
} krown_key_type_t;
```

### Fonctions principales

#### `prepare_vm_for_krown()`

Fonction principale qui prépare **automatiquement et complètement** la VM pour Krown. Cette fonction fait tout en une seule fois, sans intervention manuelle.

```c
krown_auth_result_t prepare_vm_for_krown(char *public_key_path, size_t path_size);
```

**Paramètres :**
- `public_key_path` : Buffer pour stocker le chemin de la clé publique (doit être alloué, min 512 octets recommandé)
- `path_size` : Taille du buffer

**Retour :** Code de retour `krown_auth_result_t` (`KROWN_AUTH_SUCCESS` si tout s'est bien passé)

**Actions effectuées automatiquement :**
1. ✅ Vérifie la présence d'OpenSSH client
2. ✅ Vérifie/crée le dossier `~/.ssh` avec permissions correctes (700)
3. ✅ Génère automatiquement les clés SSH si elles n'existent pas (ED25519 en priorité, RSA 4096 en fallback)
4. ✅ Vérifie et corrige automatiquement les permissions des clés existantes (600 pour privée, 644 pour publique)
5. ✅ Vérifie l'intégrité et la lisibilité des clés
6. ✅ Régénère les clés si elles sont corrompues ou illisibles
7. ✅ Retourne le chemin de la clé publique prête à être utilisée

**Note :** Cette fonction est conçue pour être appelée une seule fois. Elle prépare tout automatiquement, que les clés existent déjà ou non.

#### `krown_generate_ssh_keys()`

Génère une paire de clés SSH.

```c
krown_auth_result_t krown_generate_ssh_keys(krown_key_type_t key_type, bool force);
```

**Paramètres :**
- `key_type` : Type de clé à générer (`KROWN_KEY_ED25519` ou `KROWN_KEY_RSA_4096`)
- `force` : Si `true`, régénère même si les clés existent

**Exemple :**
```c
// Générer une clé ED25519 (ne fait rien si elle existe déjà)
krown_generate_ssh_keys(KROWN_KEY_ED25519, false);

// Forcer la régénération
krown_generate_ssh_keys(KROWN_KEY_ED25519, true);
```

#### `krown_keys_exist()`

Vérifie si une paire de clés existe déjà.

```c
bool krown_keys_exist(krown_key_type_t key_type);
```

**Exemple :**
```c
if (krown_keys_exist(KROWN_KEY_ED25519)) {
    printf("Clé ED25519 existe déjà\n");
}
```

#### `krown_get_public_key()`

Lit le contenu de la clé publique.

```c
krown_auth_result_t krown_get_public_key(
    krown_key_type_t key_type, 
    char *buffer, 
    size_t buffer_size
);
```

**Exemple :**
```c
char public_key[8192];
if (krown_get_public_key(KROWN_KEY_ED25519, public_key, sizeof(public_key)) 
    == KROWN_AUTH_SUCCESS) {
    printf("Clé publique: %s\n", public_key);
}
```

#### `krown_get_public_key_path()`

Obtient le chemin complet de la clé publique.

```c
krown_auth_result_t krown_get_public_key_path(
    krown_key_type_t key_type, 
    char *buffer, 
    size_t buffer_size
);
```

#### `krown_check_openssh_client()`

Vérifie si OpenSSH client est disponible.

```c
bool krown_check_openssh_client(void);
```

#### `krown_ensure_ssh_directory()`

Vérifie et crée le dossier `.ssh` si nécessaire.

```c
krown_auth_result_t krown_ensure_ssh_directory(void);
```

#### `krown_auth_get_error_message()`

Obtient un message d'erreur descriptif.

```c
const char *krown_auth_get_error_message(krown_auth_result_t result);
```

**Exemple :**
```c
krown_auth_result_t result = prepare_vm_for_krown(path, size);
if (result != KROWN_AUTH_SUCCESS) {
    fprintf(stderr, "Erreur: %s\n", krown_auth_get_error_message(result));
}
```

## 📖 Exemples

### Exemple 1 : Préparation basique

```c
#include "krown_auth.h"
#include <stdio.h>

int main(void) {
    char public_key_path[512];
    
    if (prepare_vm_for_krown(public_key_path, sizeof(public_key_path)) 
        == KROWN_AUTH_SUCCESS) {
        printf("Clé publique disponible: %s\n", public_key_path);
        return 0;
    }
    
    return 1;
}
```

### Exemple 2 : Génération conditionnelle

```c
#include "krown_auth.h"
#include <stdio.h>

int main(void) {
    // Vérifier si les clés existent
    if (!krown_keys_exist(KROWN_KEY_ED25519)) {
        printf("Génération des clés SSH...\n");
        
        krown_auth_result_t result = krown_generate_ssh_keys(
            KROWN_KEY_ED25519, 
            false
        );
        
        if (result != KROWN_AUTH_SUCCESS) {
            printf("Erreur: %s\n", krown_auth_get_error_message(result));
            return 1;
        }
        
        printf("Clés générées avec succès\n");
    } else {
        printf("Clés déjà présentes\n");
    }
    
    // Lire la clé publique
    char public_key[8192];
    if (krown_get_public_key(KROWN_KEY_ED25519, public_key, sizeof(public_key)) 
        == KROWN_AUTH_SUCCESS) {
        printf("Clé publique:\n%s\n", public_key);
    }
    
    return 0;
}
```

### Exemple 3 : Gestion d'erreurs complète

```c
#include "krown_auth.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // Vérifier OpenSSH
    if (!krown_check_openssh_client()) {
        fprintf(stderr, "Erreur: OpenSSH client non trouvé\n");
        fprintf(stderr, "Installez OpenSSH et réessayez\n");
        return 1;
    }
    
    // Vérifier/créer le dossier .ssh
    krown_auth_result_t result = krown_ensure_ssh_directory();
    if (result != KROWN_AUTH_SUCCESS) {
        fprintf(stderr, "Erreur dossier .ssh: %s\n", 
                krown_auth_get_error_message(result));
        return 1;
    }
    
    // Générer les clés
    result = krown_generate_ssh_keys(KROWN_KEY_ED25519, false);
    if (result == KROWN_AUTH_SUCCESS) {
        printf("✓ Clés SSH prêtes\n");
    } else if (result == KROWN_AUTH_ERROR_KEY_GEN) {
        // Essayer RSA en fallback
        printf("ED25519 non disponible, essai avec RSA 4096...\n");
        result = krown_generate_ssh_keys(KROWN_KEY_RSA_4096, false);
    }
    
    if (result != KROWN_AUTH_SUCCESS) {
        fprintf(stderr, "Erreur: %s\n", krown_auth_get_error_message(result));
        return 1;
    }
    
    // Obtenir le chemin de la clé publique
    char public_key_path[512];
    result = krown_get_public_key_path(KROWN_KEY_ED25519, 
                                       public_key_path, 
                                       sizeof(public_key_path));
    if (result == KROWN_AUTH_SUCCESS) {
        printf("Chemin: %s\n", public_key_path);
    }
    
    return 0;
}
```

## 🔧 Compatibilité

### Systèmes d'exploitation

- ✅ **Linux** : Testé sur Ubuntu, Debian, CentOS
- ✅ **macOS** : Compatible (testé sur macOS 10.15+)
- ⚠️ **Windows** : Compatible via MinGW/MSYS ou WSL

### Compilateurs

- ✅ GCC 4.9+
- ✅ Clang 3.5+
- ⚠️ MSVC (avec limitations pour certaines fonctions Unix)

## 🔒 Sécurité

### Permissions

Le module applique automatiquement les permissions de sécurité recommandées :

| Fichier/Dossier | Permissions | Description |
|----------------|-------------|-------------|
| `~/.ssh` | `700` (drwx------) | Dossier accessible uniquement par le propriétaire |
| Clé privée | `600` (-rw-------) | Lisible/écritable uniquement par le propriétaire |
| Clé publique | `644` (-rw-r--r--) | Lisible par tous, modifiable par le propriétaire |

### Emplacement des clés

Les clés sont stockées dans le dossier home de l'utilisateur :

- **ED25519** :
  - Privée : `~/.ssh/id_ed25519`
  - Publique : `~/.ssh/id_ed25519.pub`
- **RSA 4096** :
  - Privée : `~/.ssh/id_rsa`
  - Publique : `~/.ssh/id_rsa.pub`

### Notes de sécurité

- ⚠️ Les clés sont générées **sans phrase de passe** (option `-N ""`)
- ✅ Les permissions sont vérifiées et corrigées automatiquement
- ✅ Le module ne modifie jamais les clés existantes sans demande explicite (`force=true`)

## 🐛 Dépannage

### Erreur : OpenSSH non trouvé

**Symptôme :** `KROWN_AUTH_ERROR_OPENSSH_NOT_FOUND`

**Solution :**
```bash
# Vérifier si ssh-keygen est disponible
which ssh-keygen

# Installer OpenSSH si nécessaire
# Ubuntu/Debian
sudo apt-get install openssh-client

# CentOS/RHEL
sudo yum install openssh-clients
```

### Erreur : Permissions du dossier .ssh

**Symptôme :** `KROWN_AUTH_ERROR_PERMISSIONS`

**Solution :**
```bash
# Vérifier les permissions
ls -ld ~/.ssh

# Corriger les permissions
chmod 700 ~/.ssh
chmod 600 ~/.ssh/id_*
chmod 644 ~/.ssh/*.pub
```

### Erreur : Génération de clés échouée

**Symptôme :** `KROWN_AUTH_ERROR_KEY_GEN`

**Solutions possibles :**
1. Vérifier que `ssh-keygen` fonctionne manuellement :
   ```bash
   ssh-keygen -t ed25519 -f /tmp/test_key -N ""
   ```
2. Vérifier les permissions du dossier `.ssh`
3. Essayer avec RSA 4096 en fallback

### Clés existantes non détectées

Si les clés existent mais ne sont pas détectées :

1. Vérifier que les fichiers existent :
   ```bash
   ls -la ~/.ssh/id_*
   ```
2. Vérifier les permissions
3. Utiliser `force=true` pour régénérer

## 📝 Codes de retour

| Code | Constante | Description |
|------|-----------|-------------|
| `0` | `KROWN_AUTH_SUCCESS` | Opération réussie |
| `-1` | `KROWN_AUTH_ERROR_SSH_DIR` | Erreur avec le dossier .ssh |
| `-2` | `KROWN_AUTH_ERROR_KEY_GEN` | Erreur lors de la génération des clés |
| `-3` | `KROWN_AUTH_ERROR_PERMISSIONS` | Erreur de permissions |
| `-4` | `KROWN_AUTH_ERROR_OPENSSH_NOT_FOUND` | OpenSSH non trouvé |
| `-5` | `KROWN_AUTH_ERROR_READ_KEY` | Erreur de lecture de clé |
| `-6` | `KROWN_AUTH_ERROR_MEMORY` | Erreur d'allocation mémoire |

## 🤝 Contribution

Les contributions sont les bienvenues ! Pour contribuer :

1. Fork le projet
2. Créer une branche pour votre fonctionnalité (`git checkout -b feature/AmazingFeature`)
3. Commit vos changements (`git commit -m 'Add some AmazingFeature'`)
4. Push vers la branche (`git push origin feature/AmazingFeature`)
5. Ouvrir une Pull Request

## 📄 Licence

Ce projet est sous licence MIT. Voir le fichier `LICENSE` pour plus de détails.

## 🙏 Remerciements

- OpenSSH pour l'outil `ssh-keygen`
- La communauté Krown pour les retours et suggestions

## 📞 Support

Pour toute question ou problème :

1. Consulter la section [Dépannage](#-dépannage)
2. Vérifier les [exemples](#-exemples)
3. Ouvrir une issue sur le dépôt du projet

---

**Dernière mise à jour :** 2024
