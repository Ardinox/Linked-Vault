#include "table.h"
#include "storage.h"

// 1. Global Head of the "List of Lists"
Table *global_tables_head = NULL;

// 2. Global Lock (Protects the list of tables itself)
pthread_mutex_t global_list_lock = PTHREAD_MUTEX_INITIALIZER;

Table* get_or_create_table(const char *table_id){
    // --- STEP A: Thread Safety ---
    pthread_mutex_lock(&global_list_lock);

    // --- STEP B: Search for existing table ---
    Table *current = global_tables_head;
    while(current!=NULL){
        if(strcmp(current->table_id, table_id)==0){
            pthread_mutex_unlock(&global_list_lock);
            return current;
        }
        current = current->next;
    }

    // --- STEP C: Create new table (The "CREATE" part) ---
    Table *new_table = (Table*)malloc(sizeof(Table));
    if(!new_table){
        printf("Failed to allocate memory\n");
        pthread_mutex_unlock(&global_list_lock);
        return NULL;
    }

    // 1. Set ID
    strncpy(new_table->table_id, table_id, 49);
    new_table->table_id[49] = '\0';

    // 2. Init the specific lock for THIS table
    if (pthread_mutex_init(&new_table->lock, NULL) != 0) {
        printf("Failed to init mutex for table %s\n", table_id);
        free(new_table);
        pthread_mutex_unlock(&global_list_lock);
        return NULL;
    }
    // 3. Init the empty list pointers (head/tail = NULL)
    init_employee_list(&new_table->employeelist);

    // 4. Load data from disk (if it exists)
    load_table_binary(new_table);

    // 5. Link to global list
    new_table->next = global_tables_head;
    global_tables_head = new_table;

    pthread_mutex_unlock(&global_list_lock);
    return new_table;
}

