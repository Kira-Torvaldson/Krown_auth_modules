#include "krown_auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#define access _access
#define mkdir(path, mode) _mkdir(path)
#define stat _stat
#define chmod _chmod
#else
#include <sys/wait.h>
#include <pwd.h>
#endif

#define SSH_DIR_PERMISSIONS 0700
#define PRIVATE_KEY_PERMISSIONS 0600
#define PUBLIC_KEY_PERMISSIONS 0644
#define MAX_PATH_LENGTH 512
#define MAX_KEY_LENGTH 8192

/**
 * @brief Obtient le chemin du dossier home de l'utilisateur
 */
static int get_home_directory(char *buffer, size_t size) {
    const char *home = getenv("HOME");
    
#ifdef _WIN32
    // Sur Windows, utiliser USERPROFILE si HOME n'est pas défini
    if (home == NULL) {
        home = getenv("USERPROFILE");
    }
    // Sinon utiliser HOMEDRIVE + HOMEPATH
    if (home == NULL) {
        const char *drive = getenv("HOMEDRIVE");
        const char *path = getenv("HOMEPATH");
        if (drive != NULL && path != NULL) {
            if (strlen(drive) + strlen(path) < size) {
                snprintf(buffer, size, "%s%s", drive, path);
                return 0;
            }
        }
    }
#else
    if (home == NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL) {
            home = pw->pw_dir;
        }
    }
#endif
    
    if (home == NULL) {
        return -1;
    }
    
    if (strlen(home) >= size) {
        return -1;
    }
    
    strncpy(buffer, home, size - 1);
    buffer[size - 1] = '\0';
    return 0;
}

/**
 * @brief Construit le chemin complet d'un fichier dans .ssh
 */
static int build_ssh_path(const char *filename, char *buffer, size_t size) {
    char home[MAX_PATH_LENGTH];
    if (get_home_directory(home, sizeof(home)) != 0) {
        return -1;
    }
    
    int ret = snprintf(buffer, size, "%s/.ssh/%s", home, filename);
    if (ret < 0 || ret >= (int)size) {
        return -1;
    }
    return 0;
}

/**
 * @brief Vérifie si un fichier existe
 */
static bool file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

/**
 * @brief Vérifie et corrige les permissions d'un fichier
 */
static int set_file_permissions(const char *path, mode_t permissions) {
#ifdef _WIN32
    // Sur Windows, chmod peut ne pas fonctionner correctement
    // On essaie quand même mais on ne retourne pas d'erreur si ça échoue
    _chmod(path, permissions);
    return 0; // On considère que c'est OK sur Windows
#else
    if (chmod(path, permissions) != 0) {
        return -1;
    }
    return 0;
#endif
}

/**
 * @brief Exécute une commande système et capture sa sortie
 */
static int execute_command(const char *command, char *output, size_t output_size) {
    // Sur Linux, popen utilise déjà le shell par défaut (/bin/sh)
    // Pas besoin de spécifier explicitement, cela fonctionne sur toutes les distributions
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        return -1;
    }
    
    if (output != NULL && output_size > 0) {
        // Utiliser fgets pour lire ligne par ligne (plus sûr)
        if (fgets(output, (int)output_size, fp) != NULL) {
            // Supprimer le saut de ligne final si présent
            size_t len = strlen(output);
            if (len > 0 && output[len - 1] == '\n') {
                output[len - 1] = '\0';
            }
        } else {
            output[0] = '\0';
        }
    } else {
        // Si on ne veut pas la sortie, juste lire pour vider le buffer
        char dummy[256];
        while (fgets(dummy, sizeof(dummy), fp) != NULL) {
            // Vider le buffer
        }
    }
    
    int status = pclose(fp);
    
#ifdef _WIN32
    // Sur Windows, pclose retourne directement le code de sortie
    // ou -1 en cas d'erreur
    if (status == -1) {
        return -1;
    }
    return status;
#else
    // Sur Unix, utiliser WIFEXITED et WEXITSTATUS
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
#endif
}

bool krown_check_openssh_client(void) {
    // Vérifier si ssh-keygen est disponible
    // On utilise une commande qui devrait toujours fonctionner si ssh-keygen existe
#ifdef _WIN32
    // Sur Windows, utiliser une commande qui redirige la sortie vers nul
    int result = execute_command("ssh-keygen --help 2>nul", NULL, 0);
#else
    // Sur Unix/Linux (Debian/Ubuntu/Arch), utiliser --help qui est universellement supporté
    // Rediriger stderr vers stdout puis vers /dev/null pour éviter les messages
    int result = execute_command("ssh-keygen --help >/dev/null 2>&1", NULL, 0);
    // Si --help ne fonctionne pas (peu probable), essayer -V (pour les versions récentes)
    if (result != 0 && result != 1) {
        result = execute_command("ssh-keygen -V >/dev/null 2>&1", NULL, 0);
    }
#endif
    // ssh-keygen retourne généralement 0 ou 1, les deux sont acceptables
    // -1 signifie que la commande n'a pas pu être exécutée (ssh-keygen n'existe pas)
    // Tout autre code d'erreur signifie aussi que ssh-keygen n'est pas disponible
    return (result == 0 || result == 1);
}

krown_auth_result_t krown_ensure_ssh_directory(void) {
    char home[MAX_PATH_LENGTH] = {0};
    if (get_home_directory(home, sizeof(home)) != 0) {
        return KROWN_AUTH_ERROR_SSH_DIR;
    }
    
    // Vérifier que home n'est pas vide
    if (home[0] == '\0') {
        return KROWN_AUTH_ERROR_SSH_DIR;
    }
    
    char ssh_dir[MAX_PATH_LENGTH] = {0};
    int ret = snprintf(ssh_dir, sizeof(ssh_dir), "%s/.ssh", home);
    if (ret < 0 || ret >= (int)sizeof(ssh_dir)) {
        return KROWN_AUTH_ERROR_SSH_DIR;
    }
    
    // Créer le dossier s'il n'existe pas
    if (!file_exists(ssh_dir)) {
        if (mkdir(ssh_dir, SSH_DIR_PERMISSIONS) != 0) {
            return KROWN_AUTH_ERROR_SSH_DIR;
        }
    } else {
        // Vérifier et corriger les permissions (uniquement sur Unix)
#ifndef _WIN32
        struct stat st;
        if (stat(ssh_dir, &st) == 0) {
            mode_t current_mode = st.st_mode & 0777;
            if (current_mode != SSH_DIR_PERMISSIONS) {
                if (chmod(ssh_dir, SSH_DIR_PERMISSIONS) != 0) {
                    return KROWN_AUTH_ERROR_PERMISSIONS;
                }
            }
        }
#endif
    }
    
    return KROWN_AUTH_SUCCESS;
}

bool krown_keys_exist(krown_key_type_t key_type) {
    char private_key_path[MAX_PATH_LENGTH];
    char public_key_path[MAX_PATH_LENGTH];
    
    const char *key_name = (key_type == KROWN_KEY_ED25519) ? "id_ed25519" : "id_rsa";
    
    if (build_ssh_path(key_name, private_key_path, sizeof(private_key_path)) != 0) {
        return false;
    }
    
    // Construire le chemin de la clé publique
    snprintf(public_key_path, sizeof(public_key_path), "%s.pub", private_key_path);
    
    return file_exists(private_key_path) && file_exists(public_key_path);
}

krown_auth_result_t krown_generate_ssh_keys(krown_key_type_t key_type, bool force) {
    // Vérifier OpenSSH
    if (!krown_check_openssh_client()) {
        return KROWN_AUTH_ERROR_OPENSSH_NOT_FOUND;
    }
    
    // S'assurer que le dossier .ssh existe
    krown_auth_result_t result = krown_ensure_ssh_directory();
    if (result != KROWN_AUTH_SUCCESS) {
        return result;
    }
    
    // Vérifier si les clés existent déjà
    if (krown_keys_exist(key_type) && !force) {
        return KROWN_AUTH_SUCCESS; // Les clés existent déjà
    }
    
    char private_key_path[MAX_PATH_LENGTH];
    const char *key_name = (key_type == KROWN_KEY_ED25519) ? "id_ed25519" : "id_rsa";
    
    if (build_ssh_path(key_name, private_key_path, sizeof(private_key_path)) != 0) {
        return KROWN_AUTH_ERROR_SSH_DIR;
    }
    
    // Construire la commande ssh-keygen
    // Sur Linux, échapper les caractères spéciaux dans le chemin si nécessaire
    char command[1024];
#ifdef _WIN32
    // Sur Windows, utiliser des guillemets doubles et rediriger stderr vers nul
    if (key_type == KROWN_KEY_ED25519) {
        snprintf(command, sizeof(command), 
                 "ssh-keygen -t ed25519 -f \"%s\" -N \"\" -q 2>nul", 
                 private_key_path);
    } else {
        snprintf(command, sizeof(command), 
                 "ssh-keygen -t rsa -b 4096 -f \"%s\" -N \"\" -q 2>nul", 
                 private_key_path);
    }
#else
    // Sur Linux/Unix, utiliser des guillemets simples pour le chemin
    // et rediriger stderr vers stdout (2>&1) puis vers /dev/null pour le mode silencieux
    if (key_type == KROWN_KEY_ED25519) {
        snprintf(command, sizeof(command), 
                 "ssh-keygen -t ed25519 -f '%s' -N '' -q >/dev/null 2>&1", 
                 private_key_path);
    } else {
        snprintf(command, sizeof(command), 
                 "ssh-keygen -t rsa -b 4096 -f '%s' -N '' -q >/dev/null 2>&1", 
                 private_key_path);
    }
#endif
    
    // Exécuter la génération
    int exit_code = execute_command(command, NULL, 0);
    if (exit_code != 0) {
        return KROWN_AUTH_ERROR_KEY_GEN;
    }
    
    // Appliquer les permissions
    if (set_file_permissions(private_key_path, PRIVATE_KEY_PERMISSIONS) != 0) {
        return KROWN_AUTH_ERROR_PERMISSIONS;
    }
    
    char public_key_path[MAX_PATH_LENGTH];
    snprintf(public_key_path, sizeof(public_key_path), "%s.pub", private_key_path);
    
    if (set_file_permissions(public_key_path, PUBLIC_KEY_PERMISSIONS) != 0) {
        return KROWN_AUTH_ERROR_PERMISSIONS;
    }
    
    return KROWN_AUTH_SUCCESS;
}

krown_auth_result_t krown_get_public_key_path(krown_key_type_t key_type, char *buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size == 0) {
        return KROWN_AUTH_ERROR_MEMORY;
    }
    
    const char *key_name = (key_type == KROWN_KEY_ED25519) ? "id_ed25519" : "id_rsa";
    char filename[MAX_PATH_LENGTH];
    snprintf(filename, sizeof(filename), "%s.pub", key_name);
    
    if (build_ssh_path(filename, buffer, buffer_size) != 0) {
        return KROWN_AUTH_ERROR_SSH_DIR;
    }
    
    return KROWN_AUTH_SUCCESS;
}

krown_auth_result_t krown_get_public_key(krown_key_type_t key_type, char *buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size == 0) {
        return KROWN_AUTH_ERROR_MEMORY;
    }
    
    char public_key_path[MAX_PATH_LENGTH];
    krown_auth_result_t result = krown_get_public_key_path(key_type, public_key_path, sizeof(public_key_path));
    if (result != KROWN_AUTH_SUCCESS) {
        return result;
    }
    
    if (!file_exists(public_key_path)) {
        return KROWN_AUTH_ERROR_READ_KEY;
    }
    
    FILE *fp = fopen(public_key_path, "r");
    if (fp == NULL) {
        return KROWN_AUTH_ERROR_READ_KEY;
    }
    
    size_t read = fread(buffer, 1, buffer_size - 1, fp);
    buffer[read] = '\0';
    
    // Supprimer le saut de ligne final si présent
    if (read > 0 && buffer[read - 1] == '\n') {
        buffer[read - 1] = '\0';
    }
    
    fclose(fp);
    return KROWN_AUTH_SUCCESS;
}

/**
 * @brief Vérifie et corrige les permissions des clés SSH existantes
 */
static krown_auth_result_t ensure_key_permissions(krown_key_type_t key_type) {
    char private_key_path[MAX_PATH_LENGTH];
    char public_key_path[MAX_PATH_LENGTH];
    const char *key_name = (key_type == KROWN_KEY_ED25519) ? "id_ed25519" : "id_rsa";
    
    if (build_ssh_path(key_name, private_key_path, sizeof(private_key_path)) != 0) {
        return KROWN_AUTH_ERROR_SSH_DIR;
    }
    
    snprintf(public_key_path, sizeof(public_key_path), "%s.pub", private_key_path);
    
    // Vérifier et corriger les permissions de la clé privée si elle existe
    if (file_exists(private_key_path)) {
        if (set_file_permissions(private_key_path, PRIVATE_KEY_PERMISSIONS) != 0) {
            return KROWN_AUTH_ERROR_PERMISSIONS;
        }
    }
    
    // Vérifier et corriger les permissions de la clé publique si elle existe
    if (file_exists(public_key_path)) {
        if (set_file_permissions(public_key_path, PUBLIC_KEY_PERMISSIONS) != 0) {
            return KROWN_AUTH_ERROR_PERMISSIONS;
        }
    }
    
    return KROWN_AUTH_SUCCESS;
}

krown_auth_result_t prepare_vm_for_krown(char *public_key_path, size_t path_size) {
    if (public_key_path == NULL || path_size == 0) {
        return KROWN_AUTH_ERROR_MEMORY;
    }
    
    // 1. Vérifier la présence d'OpenSSH client
    if (!krown_check_openssh_client()) {
        return KROWN_AUTH_ERROR_OPENSSH_NOT_FOUND;
    }
    
    // 2. Vérifier les permissions et l'intégrité de ~/.ssh
    krown_auth_result_t result = krown_ensure_ssh_directory();
    if (result != KROWN_AUTH_SUCCESS) {
        return result;
    }
    
    // 3. Déterminer quel type de clé utiliser et GARANTIR qu'au moins une clé existe
    krown_key_type_t key_type = KROWN_KEY_ED25519;
    bool keys_exist = krown_keys_exist(key_type);
    
    // 4. Si aucune clé ED25519 n'existe, la créer
    if (!keys_exist) {
        result = krown_generate_ssh_keys(key_type, false);
        if (result != KROWN_AUTH_SUCCESS) {
            // Si ED25519 échoue, essayer RSA 4096
            key_type = KROWN_KEY_RSA_4096;
            keys_exist = krown_keys_exist(key_type);
            
            // Si RSA n'existe pas non plus, le créer OBLIGATOIREMENT
            if (!keys_exist) {
                result = krown_generate_ssh_keys(key_type, false);
                if (result != KROWN_AUTH_SUCCESS) {
                    return result; // Erreur : impossible de créer une clé
                }
            }
        }
    }
    
    // 5. Vérifier que la clé existe vraiment (sécurité)
    if (!krown_keys_exist(key_type)) {
        // Si la clé n'existe toujours pas après génération, essayer RSA en dernier recours
        if (key_type == KROWN_KEY_ED25519) {
            key_type = KROWN_KEY_RSA_4096;
            if (!krown_keys_exist(key_type)) {
                result = krown_generate_ssh_keys(key_type, false);
                if (result != KROWN_AUTH_SUCCESS) {
                    return result;
                }
            }
        } else {
            // Si RSA n'existe pas non plus après génération, erreur
            return KROWN_AUTH_ERROR_KEY_GEN;
        }
    }
    
    // 6. Vérifier que la clé est lisible (intégrité)
    char test_buffer[256];
    if (krown_get_public_key(key_type, test_buffer, sizeof(test_buffer)) != KROWN_AUTH_SUCCESS) {
        // Clé corrompue ou illisible, régénérer
        result = krown_generate_ssh_keys(key_type, true);
        if (result != KROWN_AUTH_SUCCESS) {
            // Si régénération échoue, essayer l'autre type de clé
            krown_key_type_t fallback_type = (key_type == KROWN_KEY_ED25519) 
                ? KROWN_KEY_RSA_4096 
                : KROWN_KEY_ED25519;
            
            if (!krown_keys_exist(fallback_type)) {
                result = krown_generate_ssh_keys(fallback_type, false);
                if (result != KROWN_AUTH_SUCCESS) {
                    return result;
                }
                key_type = fallback_type;
            } else {
                key_type = fallback_type;
            }
        }
    }
    
    // 7. Vérifier et corriger les permissions des clés (même si elles existaient déjà)
    result = ensure_key_permissions(key_type);
    if (result != KROWN_AUTH_SUCCESS) {
        return result;
    }
    
    // 8. Exposer le chemin de la clé publique (garantie d'exister et d'être valide à ce stade)
    result = krown_get_public_key_path(key_type, public_key_path, path_size);
    return result;
}

void krown_auth_cleanup(krown_ssh_keys_t *keys) {
    if (keys == NULL) {
        return;
    }
    
    if (keys->private_key_path != NULL) {
        free(keys->private_key_path);
        keys->private_key_path = NULL;
    }
    
    if (keys->public_key_path != NULL) {
        free(keys->public_key_path);
        keys->public_key_path = NULL;
    }
    
    if (keys->public_key_content != NULL) {
        free(keys->public_key_content);
        keys->public_key_content = NULL;
    }
}

const char *krown_auth_get_error_message(krown_auth_result_t result) {
    switch (result) {
        case KROWN_AUTH_SUCCESS:
            return "Succès";
        case KROWN_AUTH_ERROR_SSH_DIR:
            return "Erreur lors de la création ou de l'accès au dossier .ssh";
        case KROWN_AUTH_ERROR_KEY_GEN:
            return "Erreur lors de la génération des clés SSH";
        case KROWN_AUTH_ERROR_PERMISSIONS:
            return "Erreur lors de la définition des permissions";
        case KROWN_AUTH_ERROR_OPENSSH_NOT_FOUND:
            return "OpenSSH client non trouvé sur le système";
        case KROWN_AUTH_ERROR_READ_KEY:
            return "Erreur lors de la lecture de la clé publique";
        case KROWN_AUTH_ERROR_MEMORY:
            return "Erreur d'allocation mémoire";
        default:
            return "Erreur inconnue";
    }
}

