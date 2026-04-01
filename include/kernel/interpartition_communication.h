#include "../arinc/arinc_apex_types.h"
#include "../arinc/arinc_interpartition_communication.h"

#ifndef INTERPARTITION
#define INTERPARTITION

struct krnl_sampling_channel {
    char *name;
    uint8_t *buffer;
    MESSAGE_SIZE_TYPE max_message_size;
    MESSAGE_SIZE_TYPE current_message_size;
    SYSTEM_TIME_TYPE last_update_time;
};

struct krnl_queuing_channel_s {
    // --- 1. La Mémoire Statique ---
    uint8_t *buffer_data;       // Le grand tableau de données brutes
    uint32_t *buffer_sizes;     // Un tableau pour retenir la taille (LENGTH) de chaque message !
    
    // --- 2. La Configuration ---
    uint32_t max_message_size;  // Taille max d'un message
    uint32_t max_nb_messages;   // Nombre max de messages dans la file (La capacité)
    
    // --- 3. L'État du Ring Buffer ---
    uint32_t current_nb_messages; // Combien de messages actuellement ?
    uint32_t read_index;          // Tête de lecture
    uint32_t write_index;         // Tête d'écriture
    
    // --- 4. La Synchronisation (Listes d'attente) ---
    struct process_s *waiting_readers; // Tête de liste des processus qui ont fait RECEIVE sur une file vide
    struct process_s *waiting_writers; // Tête de liste des processus qui ont fait SEND sur une file pleine
};

#endif