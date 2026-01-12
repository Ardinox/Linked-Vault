#include "employee.h"
#include "storage.h"

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

void save_table_binary(Table *t)
{
    if (t == NULL)
    {
        return;
    }

    // 1. Generate filename based on Table ID
    char filename[60];
    snprintf(filename, sizeof(filename), "bin/%s.bin", t->table_id);

    // 2. Open file for Writing Binary ("wb")
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        perror("Failed to open file for writing");
        return;
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
    printf("Table '%s' saved to disk.\n", t->table_id);
}

// Loads data from a binary file into the table's linked list
void load_table_binary(Table *t)
{
    // 1. Generate filename
    char filename[60];
    snprintf(filename, sizeof(filename), "bin/%s.bin", t->table_id);

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