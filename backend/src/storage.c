// File_Name storage.c
// Handles the binary database logics

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "employee.h"
#include "storage.h"
#include "table.h"

// Protects users.bin and tables_registry.bin operations (exists in main.c)
extern pthread_mutex_t file_registry_lock;

// --- Helper Functions ---
EmployeeRecord node_to_record(emp *node)
{
    EmployeeRecord rec;
    // Zero out the memory to avoid writing garbage bytes from stack
    memset(&rec, 0, sizeof(EmployeeRecord));
    if (node != NULL)
    {
        rec.id = node->id;

        strncpy(rec.name, node->name, 49);
        rec.name[49] = '\0';

        rec.age = node->age;

        strncpy(rec.department, node->department, 49);
        rec.department[49] = '\0';

        rec.salary = node->salary;
    }
    return rec;
}
emp *record_to_node(EmployeeRecord record)
{
    emp *new_node = (emp *)calloc(1, sizeof(emp));
    if (!new_node)
    {
        return NULL;
    }
    new_node->id = record.id;
    strncpy(new_node->name, record.name, 49);
    new_node->age = record.age;
    strncpy(new_node->department, record.department, 49);
    new_node->salary = record.salary;

    new_node->next = NULL;

    return new_node;
}

// --- TABLE FUNCTIONS ---
void save_table_binary(Table *t)
{
    if (t == NULL)
    {
        return;
    }

    // 1. Generate filename based on Table ID
    char filename[100];
    snprintf(filename, sizeof(filename), "bin/tables/%d.bin", t->id);

    // 2. Open file for Writing Binary ("wb")
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        // Try creating the folder if it fails (system command)
        system("mkdir -p bin/tables");
        fp = fopen(filename, "wb");
        if (fp == NULL)
        {
            perror("Failed to open file for writing");
            return;
        }
    }
    // 3. Traverse the list and write records
    emp *current = t->employeelist.head;
    while (current != NULL)
    {
        EmployeeRecord rec = node_to_record(current);
        fwrite(&rec, sizeof(EmployeeRecord), 1, fp);
        current = current->next;
    }
    fclose(fp);
    printf("Table '%d' saved to disk.\n", t->id);
}

// Loads data from a binary file into the table's linked list
void load_table_binary(Table *t)
{
    // 1. Generate filename
    char filename[100];
    snprintf(filename, sizeof(filename), "bin/tables/%d.bin", t->id);

    // 2. Open file for Reading Binary ("rb")
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        return;
    }
    EmployeeRecord rec;
    while (fread(&rec, sizeof(EmployeeRecord), 1, fp))
    {
        emp *new_node = record_to_node(rec);
        if (new_node == NULL)
            continue;

        if (t->employeelist.head == NULL)
        {
            t->employeelist.head = new_node;
            t->employeelist.tail = new_node;
        }
        else
        {
            t->employeelist.tail->next = new_node;
            t->employeelist.tail = new_node;
        }
    }
    fclose(fp);
}

// --- USER FUNCTIONS ---
User find_user_by_name(const char* username){
    FILE *fp = fopen("bin/users/users.bin", "rb");
    User user;
    User not_found = {-1, "", ""};
    if (!fp) return not_found;

    while (fread(&user, sizeof(User), 1, fp)) {
        if (strcmp(user.username, username) == 0) {
            fclose(fp);
            return user;
        }
    }
    fclose(fp);
    return not_found;
}

int save_new_user(const char* username, const char *hash){

    pthread_mutex_lock(&file_registry_lock);

    // A. Check for Duplicates
    User check = find_user_by_name(username);
    if (check.id != -1) {
        pthread_mutex_unlock(&file_registry_lock);
        return -1;
    }

    // B. Auto-Increment ID
    int max_id = 0;
    FILE *fp_read = fopen("bin/users/users.bin", "rb");
    if (fp_read) {
        User temp;
        while (fread(&temp, sizeof(User), 1, fp_read)) {
            if (temp.id > max_id) max_id = temp.id;
        }
        fclose(fp_read);
    }

    // C. Write New User
    // Ensure folder exists
    FILE *fp_write = fopen("bin/users/users.bin", "ab");
    if (!fp_write) {
        system("mkdir -p bin/users"); // Helper to create folder if missing
        fp_write = fopen("bin/users/users.bin", "ab");
        if(!fp_write) {
            pthread_mutex_unlock(&file_registry_lock);
            return -2;
        }
    }

    User new_user;
    new_user.id = max_id + 1;
    strncpy(new_user.username, username, 49);
    strncpy(new_user.password_hash, hash, 127);

    fwrite(&new_user, sizeof(User), 1, fp_write);
    fclose(fp_write);
    
    printf("[STORAGE] Created User: %s (ID: %d)\n", username, new_user.id);
    pthread_mutex_unlock(&file_registry_lock);
    return new_user.id;
}

// --- TABLE REGISTRY FUNCTIONS ---

int save_table_metadata(int owner_id, const char* display_name) {
    
    pthread_mutex_lock(&file_registry_lock);
    // A. Internal ID Logic
    int max_id = 1000;
    FILE *fp_read = fopen("bin/users/tables_registry.bin", "rb");
    if (fp_read) {
        TableMetadata temp;
        while (fread(&temp, sizeof(TableMetadata), 1, fp_read)) {
            if (temp.id > max_id) max_id = temp.id;
        }
        fclose(fp_read);
    }

    // B. Save Metadata
    FILE *fp_write = fopen("bin/users/tables_registry.bin", "ab");
    if (!fp_write) {
        system("mkdir -p bin/users");
        fp_write = fopen("bin/users/tables_registry.bin", "ab");
        if(!fp_write) {
             pthread_mutex_unlock(&file_registry_lock);
             return -1;
        }
    }

    TableMetadata meta;
    meta.id = max_id + 1;
    meta.owner_id = owner_id;
    meta.is_active = 1;
    strncpy(meta.table_name, display_name, 49);

    fwrite(&meta, sizeof(TableMetadata), 1, fp_write);
    fclose(fp_write);

    printf("[STORAGE] Registered Table: '%s' (ID: %d) for User %d\n", 
           display_name, meta.id, owner_id);
           
    pthread_mutex_unlock(&file_registry_lock);
    return meta.id;
}

TableMetadata* get_user_tables(int user_id, int *count) {
    *count = 0;

    pthread_mutex_lock(&file_registry_lock);
    FILE *fp = fopen("bin/users/tables_registry.bin", "rb");
    if (!fp) {
        pthread_mutex_unlock(&file_registry_lock);
        return NULL;
    }

    // A. Count
    TableMetadata temp;
    int matches = 0;
    while (fread(&temp, sizeof(TableMetadata), 1, fp)) {
        if (temp.owner_id == user_id && temp.is_active == 1) {
            matches++;
        }
    }
    
    if (matches == 0) {
        fclose(fp);
        pthread_mutex_unlock(&file_registry_lock);
        return NULL;
    }

    // B. Allocate & Fill
    TableMetadata *list = (TableMetadata*)malloc(matches * sizeof(TableMetadata));
    rewind(fp);

    int index = 0;
    while (fread(&temp, sizeof(TableMetadata), 1, fp)) {
        if (temp.owner_id == user_id && temp.is_active == 1) {
            if (index < matches) {
                list[index++] = temp;
            }
        }
    }
    fclose(fp);

    pthread_mutex_unlock(&file_registry_lock);
    
    *count = matches;
    return list;
}

int is_table_owner(int user_id, int table_id) {
    FILE *fp = fopen("bin/users/tables_registry.bin", "rb");
    if (!fp) return 0;

    TableMetadata temp;
    int found = 0;
    while (fread(&temp, sizeof(TableMetadata), 1, fp)) {
        if (temp.id == table_id && temp.owner_id == user_id && temp.is_active == 1) {
            found = 1;
            break;
        }
    }
    fclose(fp);
    return found;
}

int delete_table_permanently(int table_id, int owner_id) {
    unload_table(table_id);
    pthread_mutex_lock(&file_registry_lock);

    // 1. Delete the physical data file (The BIN with employee data)
    char filename[100];
    snprintf(filename, sizeof(filename), "bin/tables/%d.bin", table_id);
    remove(filename); // Returns non-zero on fail, but we continue anyway to clean registry

    // 2. Rewrite Registry to remove metadata (Copy-Swap Method)
    FILE *fp = fopen("bin/users/tables_registry.bin", "rb");
    if (!fp) {
        pthread_mutex_unlock(&file_registry_lock);
        return -1; // Registry missing
    }

    FILE *temp = fopen("bin/users/temp_registry.bin", "wb");
    if (!temp) {
        fclose(fp);
        pthread_mutex_unlock(&file_registry_lock);
        return -2; // Write fail
    }

    TableMetadata meta;
    int found = 0;
    while(fread(&meta, sizeof(TableMetadata), 1, fp)) {
        // If ID matches AND Owner matches, we SKIP writing it (Deleting it)
        if (meta.id == table_id && meta.owner_id == owner_id) {
            found = 1;
            continue; 
        }
        fwrite(&meta, sizeof(TableMetadata), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    // 3. Swap Files
    if (found) {
        remove("bin/users/tables_registry.bin");
        rename("bin/users/temp_registry.bin", "bin/users/tables_registry.bin");
        printf("[STORAGE] Table %d deleted permanently.\n", table_id);
    } else {
        remove("bin/users/temp_registry.bin"); // Cleanup temp if nothing found
    }

    pthread_mutex_unlock(&file_registry_lock);
    return found ? 0 : -3; // 0 success, -3 not found
}