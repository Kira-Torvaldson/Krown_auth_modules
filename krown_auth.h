#ifndef KROWN_AUTH_H
#define KROWN_AUTH_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Structure pour stocker les informations de clé SSH
 */
typedef struct {
    char *private_key_path;
    char *public_key_path;
    char *public_key_content;
    bool exists;
} krown_ssh_keys_t;

/**
 * @brief Résultat des opérations
 */
typedef enum {
    KROWN_AUTH_SUCCESS = 0,
    KROWN_AUTH_ERROR_SSH_DIR = -1,
    KROWN_AUTH_ERROR_KEY_GEN = -2,
    KROWN_AUTH_ERROR_PERMISSIONS = -3,
    KROWN_AUTH_ERROR_OPENSSH_NOT_FOUND = -4,
    KROWN_AUTH_ERROR_READ_KEY = -5,
    KROWN_AUTH_ERROR_MEMORY = -6
} krown_auth_result_t;

/**
 * @brief Type de clé SSH
 */
typedef enum {
    KROWN_KEY_ED25519 = 0,
    KROWN_KEY_RSA_4096 = 1
} krown_key_type_t;

/**
 * @brief Vérifie si une paire de clés SSH existe déjà
 * 
 * @param key_type Type de clé à vérifier (ED25519 ou RSA)
 * @return true si les clés existent, false sinon
 */
bool krown_keys_exist(krown_key_type_t key_type);

/**
 * @brief Génère une paire de clés SSH
 * 
 * @param key_type Type de clé à générer (ED25519 en priorité, RSA 4096 en fallback)
 * @param force Si true, régénère même si les clés existent
 * @return krown_auth_result_t Code de retour (KROWN_AUTH_SUCCESS en cas de succès)
 */
krown_auth_result_t krown_generate_ssh_keys(krown_key_type_t key_type, bool force);

/**
 * @brief Lit le contenu de la clé publique
 * 
 * @param key_type Type de clé à lire
 * @param buffer Buffer pour stocker le contenu (doit être alloué par l'appelant)
 * @param buffer_size Taille du buffer
 * @return krown_auth_result_t Code de retour
 */
krown_auth_result_t krown_get_public_key(krown_key_type_t key_type, char *buffer, size_t buffer_size);

/**
 * @brief Obtient le chemin complet de la clé publique
 * 
 * @param key_type Type de clé
 * @param buffer Buffer pour stocker le chemin (doit être alloué par l'appelant)
 * @param buffer_size Taille du buffer
 * @return krown_auth_result_t Code de retour
 */
krown_auth_result_t krown_get_public_key_path(krown_key_type_t key_type, char *buffer, size_t buffer_size);

/**
 * @brief Vérifie la présence d'OpenSSH client sur le système
 * 
 * @return true si OpenSSH est disponible, false sinon
 */
bool krown_check_openssh_client(void);

/**
 * @brief Vérifie et corrige les permissions du dossier .ssh
 * 
 * @return krown_auth_result_t Code de retour
 */
krown_auth_result_t krown_ensure_ssh_directory(void);

/**
 * @brief Fonction principale pour préparer automatiquement la VM pour Krown
 * 
 * Cette fonction prépare complètement la VM pour Krown de manière automatique :
 * - Vérifie la présence d'OpenSSH client
 * - Vérifie/crée le dossier ~/.ssh avec permissions correctes (700)
 * - Génère automatiquement les clés SSH si elles n'existent pas (ED25519 en priorité, RSA 4096 en fallback)
 * - Vérifie et corrige automatiquement les permissions des clés existantes (600 pour privée, 644 pour publique)
 * - Vérifie l'intégrité et la lisibilité des clés
 * - Régénère les clés si elles sont corrompues ou illisibles
 * - Retourne le chemin de la clé publique prête à être utilisée
 * 
 * Cette fonction est conçue pour être appelée une seule fois et prépare tout automatiquement.
 * Aucune intervention manuelle n'est nécessaire.
 * 
 * @param public_key_path Buffer pour stocker le chemin de la clé publique (doit être alloué, min 512 octets recommandé)
 * @param path_size Taille du buffer pour le chemin
 * @return krown_auth_result_t Code de retour (KROWN_AUTH_SUCCESS si tout s'est bien passé)
 */
krown_auth_result_t prepare_vm_for_krown(char *public_key_path, size_t path_size);

/**
 * @brief Libère les ressources allouées par le module
 * 
 * @param keys Structure de clés à libérer (peut être NULL)
 */
void krown_auth_cleanup(krown_ssh_keys_t *keys);

/**
 * @brief Obtient un message d'erreur descriptif
 * 
 * @param result Code de retour
 * @return const char* Message d'erreur
 */
const char *krown_auth_get_error_message(krown_auth_result_t result);

#endif /* KROWN_AUTH_H */

