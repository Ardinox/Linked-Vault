// File_Name employee.c
// Creates the Data

#include "employee.h"

// Initializing my_list with NULL
EmployeeList my_list = {NULL, NULL};

pthread_mutex_t list_lock;

// init function of mutex lock
void init_list_mutex(){
  if(pthread_mutex_init(&list_lock, NULL) != 0){
    printf("Mutex init failed\n");
    exit(1);
  }
}

// --- Create Node from JSON ---
emp *create_node_from_json(cJSON *json)
{
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

  // Assigning values to the nodes's data field
  if (j_id)
  {
    new_node->id = j_id->valueint;
  }
  if (j_name)
  {
    strncpy(new_node->name, j_name->valuestring, 49);
  }
  if (j_age)
  {
    new_node->age = j_age->valueint;
  }
  if (j_dept)
  {
    strncpy(new_node->department, j_dept->valuestring, 49);
  }
  if (j_salary)
  {
    new_node->salary = j_salary->valueint;
  }

  // Assigning values to the nodes's link field (as NULL)
  new_node->next = NULL;
  return new_node;
}