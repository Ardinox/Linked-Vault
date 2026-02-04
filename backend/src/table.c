// File_Name table.c
#include "table.h"
#include "storage.h"

// Global Head of the "List of Lists"
Table *global_tables_head = NULL;

// Global Lock
pthread_mutex_t global_list_lock = PTHREAD_MUTEX_INITIALIZER;

Table *get_or_load_table(int table_id, int owner_id)
{

    // --- STEP A: Thread Safety ---
    int lock_res = pthread_mutex_lock(&global_list_lock);

    // --- STEP B: Search RAM ---
    Table *current = global_tables_head;
    while (current != NULL)
    {
        if (current->id == table_id)
        {
            if (current->owner_id == owner_id)
            {
                pthread_mutex_unlock(&global_list_lock);
                return current;
            }
            else
            {
                pthread_mutex_unlock(&global_list_lock);
                return NULL; // Access Denied
            }
        }
        current = current->next;
    }

    // --- STEP C: Not found in RAM, Create New ---
    Table *new_table = (Table *)malloc(sizeof(Table));
    if (!new_table)
    {
        pthread_mutex_unlock(&global_list_lock);
        return NULL;
    }

    new_table->id = table_id;
    new_table->owner_id = owner_id;
    // STEP D: Initialize table-specific lock
    if (pthread_mutex_init(&new_table->lock, NULL) != 0)
    {
        free(new_table);
        pthread_mutex_unlock(&global_list_lock);
        return NULL;
    }

    init_employee_list(&new_table->employeelist);

    // Load data from disk
    load_table_binary(new_table);

    // Add to head of global list
    new_table->next = global_tables_head;
    global_tables_head = new_table;

    pthread_mutex_unlock(&global_list_lock);

    return new_table;
}

void unload_table(int table_id)
{
    pthread_mutex_lock(&global_list_lock);
    Table *curr = global_tables_head;
    Table *prev = NULL;
    while (curr != NULL)
    {
        if (curr->id == table_id)
        {
            // Unlink from list
            if (prev == NULL)
                global_tables_head = curr->next;
            else
                prev->next = curr->next;

            // Cleanup
            emp *e_curr = curr->employeelist.head;
            while (e_curr)
            {
                emp *n = e_curr->next;
                free(e_curr);
                e_curr = n;
            }
            // Cleanup table resources
            pthread_mutex_destroy(&curr->lock);
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    pthread_mutex_unlock(&global_list_lock);
}