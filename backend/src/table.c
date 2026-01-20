// File_Name table.c
// Manages the Runtime Linked List of Tables

#include "table.h"
#include "storage.h"

// 1. Global Head of the "List of Lists" (The Tables currently in RAM)
Table *global_tables_head = NULL;

// 2. Global Lock (Protects the list of tables itself)
pthread_mutex_t global_list_lock = PTHREAD_MUTEX_INITIALIZER;

// Find a table in RAM, or load it from disk if valid
Table* get_or_load_table(int table_id, int owner_id){
    
    // --- STEP A: Thread Safety ---
    pthread_mutex_lock(&global_list_lock);

    // --- STEP B: Search for existing table in RAM ---
    Table *current = global_tables_head;
    while(current != NULL){
        if(current->id == table_id){
            // SECURITY CHECK: Does this loaded table belong to the requester?
            if (current->owner_id == owner_id) {
                pthread_mutex_unlock(&global_list_lock);
                return current;
            } else {
                // Table exists but belongs to someone else!
                pthread_mutex_unlock(&global_list_lock);
                printf("[SECURITY] User %d tried to access Table %d owned by %d\n", owner_id, table_id, current->owner_id);
                return NULL; 
            }
        }
        current = current->next;
    }

    // --- STEP C: Load/Create the Table Structure ---
    // If we are here, the table is not in RAM. 
    // We trust the caller (handler.c) has already verified the ID exists in the DB via is_table_owner()
    
    Table *new_table = (Table*)malloc(sizeof(Table));
    if(!new_table){
        printf("Failed to allocate memory for table\n");
        pthread_mutex_unlock(&global_list_lock);
        return NULL;
    }

    // 1. Set Identifiers
    new_table->id = table_id;
    new_table->owner_id = owner_id;
    // (Optional: You could load the display_name from DB here if needed for UI)

    // 2. Init the specific lock for THIS table
    if (pthread_mutex_init(&new_table->lock, NULL) != 0) {
        printf("Failed to init mutex for table %d\n", table_id);
        free(new_table);
        pthread_mutex_unlock(&global_list_lock);
        return NULL;
    }

    // 3. Init the empty employee list pointers
    init_employee_list(&new_table->employeelist);

    // 4. Load actual data from disk
    // This function (in storage.c) uses the integer ID to find the file (e.g. "bin/tables/1001.bin")
    load_table_binary(new_table);

    // 5. Link to global list
    new_table->next = global_tables_head;
    global_tables_head = new_table;

    pthread_mutex_unlock(&global_list_lock);
    return new_table;
}

// Optional: Helper to unload a table (if you ever implement memory management)
void unload_table(int table_id) {
    pthread_mutex_lock(&global_list_lock);
    
    Table *curr = global_tables_head;
    Table *prev = NULL;

    while (curr != NULL) {
        if (curr->id == table_id) {
            if (prev == NULL) {
                global_tables_head = curr->next;
            } else {
                prev->next = curr->next;
            }
            
            // Free the table resources
            // (Note: You would also need to free the employee nodes here to avoid leaks)
            pthread_mutex_destroy(&curr->lock);
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    pthread_mutex_unlock(&global_list_lock);
}