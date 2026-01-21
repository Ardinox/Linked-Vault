// File_Name employee.c
// Creates the Data

#include "employee.h"

void init_employee_list(EmployeeList *list)
{
  if (list != NULL)
  {
    list->head = NULL;
    list->tail = NULL;
  }
}

// --- Create Node from JSON ---
emp *create_node_from_json(cJSON *json)
{
  if (!json) return NULL; // Safety check

  // Alloting Memory space for new node
  emp *new_node = (emp *)calloc(1, sizeof(emp));
  if (!new_node)
    return NULL;

  // Extract fields safely from JSON
  cJSON *j_id = cJSON_GetObjectItem(json, "id");
  cJSON *j_name = cJSON_GetObjectItem(json, "name");
  cJSON *j_age = cJSON_GetObjectItem(json, "age");
  cJSON *j_dept = cJSON_GetObjectItem(json, "department");
  cJSON *j_salary = cJSON_GetObjectItem(json, "salary");

  // Assigning values (Safely!)
  // We use 'valueint' for numbers and check 'valuestring' for strings
  if (j_id) {
    new_node->id = j_id->valueint;
  }
  
  if (j_name && j_name->valuestring) { // Check valuestring is not NULL
    strncpy(new_node->name, j_name->valuestring, 49);
    new_node->name[49] = '\0'; // Ensure null-termination
  }
  
  if (j_age) {
    new_node->age = j_age->valueint;
  }
  
  if (j_dept && j_dept->valuestring) { // Check valuestring is not NULL
    strncpy(new_node->department, j_dept->valuestring, 49);
    new_node->department[49] = '\0';
  }
  
  if (j_salary) {
    new_node->salary = j_salary->valueint;
  }

  new_node->next = NULL;
  return new_node;
}