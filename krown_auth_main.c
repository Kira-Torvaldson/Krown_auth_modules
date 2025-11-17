#include "krown_auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char public_key_path[512];
    char public_key_content[8192];
    krown_auth_result_t result;
    
    // Afficher l'en-tête
    printf("=== Krown Auth - Préparation de la VM pour Krown ===\n\n");
    
    // Préparer la VM et créer les clés automatiquement
    result = prepare_vm_for_krown(public_key_path, sizeof(public_key_path));
    
    if (result == KROWN_AUTH_SUCCESS) {
        printf("✓ VM préparée avec succès !\n\n");
        printf("Chemin de la clé publique: %s\n\n", public_key_path);
        
        // Déterminer le type de clé créée
        krown_key_type_t key_type = KROWN_KEY_ED25519;
        if (!krown_keys_exist(key_type)) {
            key_type = KROWN_KEY_RSA_4096;
        }
        
        // Lire et afficher le contenu de la clé publique
        result = krown_get_public_key(key_type, public_key_content, sizeof(public_key_content));
        if (result == KROWN_AUTH_SUCCESS) {
            printf("Clé publique (à utiliser avec l'API Krown):\n");
            printf("%s\n\n", public_key_content);
        } else {
            printf("⚠ Avertissement: Impossible de lire le contenu de la clé\n");
            printf("   Mais la clé existe bien à: %s\n\n", public_key_path);
        }
        
        printf("✓ La VM est maintenant prête pour Krown !\n");
        printf("✓ Les clés SSH ont été générées et configurées correctement.\n");
        
        return 0;
    } else {
        printf("✗ Erreur lors de la préparation de la VM\n");
        printf("  %s\n\n", krown_auth_get_error_message(result));
        
        // Afficher des conseils selon le type d'erreur
        if (result == KROWN_AUTH_ERROR_OPENSSH_NOT_FOUND) {
            printf("Conseil: Installez OpenSSH client:\n");
            printf("  - Ubuntu/Debian: sudo apt-get install openssh-client\n");
            printf("  - CentOS/RHEL: sudo yum install openssh-clients\n");
            printf("  - macOS: OpenSSH est inclus par défaut\n");
        } else if (result == KROWN_AUTH_ERROR_SSH_DIR) {
            printf("Conseil: Vérifiez les permissions du dossier home\n");
        } else if (result == KROWN_AUTH_ERROR_KEY_GEN) {
            printf("Conseil: Vérifiez que ssh-keygen fonctionne correctement\n");
            printf("  Test: ssh-keygen -V\n");
        }
        
        return 1;
    }
}

