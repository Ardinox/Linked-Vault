// File_Name core.handler.c
// Serves the logic for core operations

#include "handlers.h"
#include "employee.h"
#include "utils.h"
#include "table.h"
#include "storage.h"
#include "logs.h"

// --- 1. Handles insertion (Supports insertion at specific position) ---
void handle_insertion(struct mg_connection *c, struct mg_http_message *hm)
{
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"error\": \"Invalid JSON\" }");
    return;
  }

  cJSON *j_table_id = cJSON_GetObjectItem(json, "table_id");
  cJSON *j_owner_id = cJSON_GetObjectItem(json, "owner_id");

  if (!j_table_id || !cJSON_IsNumber(j_table_id) || !j_owner_id || !cJSON_IsNumber(j_owner_id))
  {

    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"error\": \"Missing/Invalid table_id or owner_id\" }");
    cJSON_Delete(json);
    return;
  }

  Table *t = get_or_load_table(j_table_id->valueint, j_owner_id->valueint);
  if (!t)
  {
    mg_http_reply(c, 403, "Access-Control-Allow-Origin: *\r\n", "{ \"error\": \"Access Denied\" }");
    cJSON_Delete(json);
    return;
  }

  cJSON *j_data = cJSON_GetObjectItem(json, "data");

  pthread_mutex_lock(&t->lock);

  const char *error_msg = validate_employee_json(j_data, t);
  if (error_msg != NULL)
  {
    pthread_mutex_unlock(&t->lock);
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"error\": \"%s\" }", error_msg);
    cJSON_Delete(json);
    return;
  }

  emp *insert = create_node_from_json(j_data);
  if (insert == NULL)
  {
    pthread_mutex_unlock(&t->lock);
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\n", "{ \"error\": \"Memory Error\" }");
    cJSON_Delete(json);
    return;
  }

  int position = -1;
  cJSON *j_pos = cJSON_GetObjectItem(json, "position");
  if (j_pos)
    position = j_pos->valueint;

  insert_node_at_pos(t, insert, position);

  save_table_binary(t);

  char log_details[128];
  snprintf(log_details, sizeof(log_details), "Added Employee: %s (ID: %d)", insert->name, insert->id);
  add_log(j_owner_id->valueint, "INSERT", log_details);

  pthread_mutex_unlock(&t->lock);

  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"success\", \"id\": %d }", insert->id);

  cJSON_Delete(json);
}

// --- Handles Display (returns all employees as a JSON Array) ---
void handle_showall(struct mg_connection *c, struct mg_http_message *hm)
{
  // Extract 'table_id' from the URL Query String
  char table_id_str[50], owner_id_str[32];
  if (mg_http_get_var(&hm->query, "table_id", table_id_str, sizeof(table_id_str)) <= 0 ||
      mg_http_get_var(&hm->query, "owner_id", owner_id_str, sizeof(owner_id_str)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"error\": \"Missing table_id or owner_id\" }");
    return;
  }

  // Get the specific Table
  Table *t = get_or_load_table(atoi(table_id_str), atoi(owner_id_str));
  if (!t)
  {
    mg_http_reply(c, 403, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"error\": \"Access Denied\" }");
    return;
  }

  cJSON *json_array = cJSON_CreateArray();

  // lock the specific table
  pthread_mutex_lock(&t->lock);

  emp *curr = t->employeelist.head;

  // Traverse the Linked List
  while (curr != NULL)
  {
    // Map struct fields to JSON keys
    cJSON *emp_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(emp_obj, "id", curr->id);
    cJSON_AddStringToObject(emp_obj, "name", curr->name);
    cJSON_AddNumberToObject(emp_obj, "age", curr->age);
    cJSON_AddStringToObject(emp_obj, "department", curr->department);
    cJSON_AddNumberToObject(emp_obj, "salary", curr->salary);

    // Add object to array
    cJSON_AddItemToArray(json_array, emp_obj);
    curr = curr->next;
  }

  // Convert JSON object to String
  char *response_str = cJSON_PrintUnformatted(json_array);

  // Unlock the list
  pthread_mutex_unlock(&t->lock);

  // Successful Response with data
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", response_str);

  // Cleanup
  free(response_str);
  cJSON_Delete(json_array);
}

// --- Handles Search by ID (Linear Search: as the data is not sorted according to ID) ---
void handle_search_by_id(struct mg_connection *c, struct mg_http_message *hm)
{
  char query_str[50], table_id_str[32], owner_id_str[32];

  if (mg_http_get_var(&hm->query, "query", query_str, sizeof(query_str)) <= 0)
  {
    if (mg_http_get_var(&hm->query, "id", query_str, sizeof(query_str)) <= 0)
    {
      // If neither exists, assume empty search (match everything? or error?)
      // Let's error for safety.
      mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                    "{ \"error\": \"Missing 'query' parameter\" }");
      return;
    }
  }

  if (mg_http_get_var(&hm->query, "table_id", table_id_str, sizeof(table_id_str)) <= 0 ||
      mg_http_get_var(&hm->query, "owner_id", owner_id_str, sizeof(owner_id_str)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"error\": \"Missing params\" }");
    return;
  }

  // Get the specific Table
  Table *t = get_or_load_table(atoi(table_id_str), atoi(owner_id_str));
  if (!t)
  {
    mg_http_reply(c, 403, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"error\": \"Access Denied\" }");
    return;
  }

  // lock the list
  pthread_mutex_lock(&t->lock);

  // Prepare JSON Array
  cJSON *results_array = cJSON_CreateArray();

  emp *curr = t->employeelist.head;
  int match_count = 0;

  // Buffer for converting numbers (ID/Age/Salary) to strings
  char temp_buf[64];

  // Linear Search through the WHOLE list
  while (curr != NULL)
  {
    int is_match = 0;

    // CHECK 1: Name (String)
    if (str_contains_ci(curr->name, query_str))
      is_match = 1;

    // CHECK 2: Department (String)
    else if (str_contains_ci(curr->department, query_str))
      is_match = 1;

    // CHECK 3: ID (Int -> String)
    if (!is_match)
    {
      snprintf(temp_buf, sizeof(temp_buf), "%d", curr->id);
      if (strstr(temp_buf, query_str))
        is_match = 1;
    }

    // CHECK 4: Age (Int -> String)
    if (!is_match)
    {
      snprintf(temp_buf, sizeof(temp_buf), "%d", curr->age);
      if (strstr(temp_buf, query_str))
        is_match = 1;
    }

    // CHECK 5: Salary (Int -> String)
    if (!is_match)
    {
      snprintf(temp_buf, sizeof(temp_buf), "%d", curr->salary);
      if (strstr(temp_buf, query_str))
        is_match = 1;
    }

    // 5. If Match Found -> Add to Array
    if (is_match)
    {
      cJSON *emp_obj = cJSON_CreateObject();
      cJSON_AddNumberToObject(emp_obj, "id", curr->id);
      cJSON_AddStringToObject(emp_obj, "name", curr->name);
      cJSON_AddNumberToObject(emp_obj, "age", curr->age);
      cJSON_AddStringToObject(emp_obj, "department", curr->department);
      cJSON_AddNumberToObject(emp_obj, "salary", curr->salary);

      cJSON_AddItemToArray(results_array, emp_obj);
      match_count++;
    }

    curr = curr->next;
  }

  // unlock the list
  pthread_mutex_unlock(&t->lock);

  // Response
  char *response_str = cJSON_PrintUnformatted(results_array);
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", response_str);

  // Cleanup
  free(response_str);
  cJSON_Delete(results_array);
}

// --- Handles Deletion (Removes a node by ID and frees its memory) ---
void handle_delete(struct mg_connection *c, struct mg_http_message *hm)
{
  char id_str[32], table_id_str[32], owner_id_str[32];

  if (mg_http_get_var(&hm->query, "id", id_str, sizeof(id_str)) <= 0 ||
      mg_http_get_var(&hm->query, "table_id", table_id_str, sizeof(table_id_str)) <= 0 ||
      mg_http_get_var(&hm->query, "owner_id", owner_id_str, sizeof(owner_id_str)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"error\": \"Missing params\" }");
    return;
  }

  Table *t = get_or_load_table(atoi(table_id_str), atoi(owner_id_str));
  if (!t)
  {
    mg_http_reply(c, 403, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"error\": \"Access Denied\" }");
    return;
  }

  int target_id = atoi(id_str);
  pthread_mutex_lock(&t->lock);

  emp *curr = t->employeelist.head;
  emp *prev = NULL;

  if (curr && curr->id == target_id)
  { // Head
    t->employeelist.head = curr->next;
    if (!t->employeelist.head)
      t->employeelist.tail = NULL;
    save_table_binary(t);

    char log_details[64];
    snprintf(log_details, sizeof(log_details), "Deleted Employee ID: %d", target_id);
    add_log(atoi(owner_id_str), "DELETE", log_details);

    pthread_mutex_unlock(&t->lock);
    free(curr);
    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Deleted\" }");
    return;
  }

  while (curr && curr->id != target_id)
  {
    prev = curr;
    curr = curr->next;
  }

  if (!curr)
  {
    pthread_mutex_unlock(&t->lock);
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"error\": \"ID not found\" }");
    return;
  }

  prev->next = curr->next;
  if (curr == t->employeelist.tail)
    t->employeelist.tail = prev;
  save_table_binary(t);

  char log_details[64];
  snprintf(log_details, sizeof(log_details), "Deleted Employee ID: %d", target_id);
  add_log(atoi(owner_id_str), "DELETE", log_details);

  pthread_mutex_unlock(&t->lock);
  free(curr);
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Deleted\" }");
}

// --- Reverse Linked List ---
void handle_reverse(struct mg_connection *c, struct mg_http_message *hm)
{
  char table_id_str[32], owner_id_str[32];

  // 1. Extract Both IDs
  if (mg_http_get_var(&hm->query, "table_id", table_id_str, sizeof(table_id_str)) <= 0 ||
      mg_http_get_var(&hm->query, "owner_id", owner_id_str, sizeof(owner_id_str)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing 'table_id' or 'owner_id' parameter\" }");
    return;
  }

  // 2. Load Table using Integers
  Table *t = get_or_load_table(atoi(table_id_str), atoi(owner_id_str));
  if (!t)
  {
    mg_http_reply(c, 403, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Access Denied or Table Not Found\" }");
    return;
  }

  // 3. Logic (Unchanged)
  pthread_mutex_lock(&t->lock);

  emp *prev = NULL, *curr = t->employeelist.head, *next = NULL;
  emp *old_head = t->employeelist.head;
  while (curr != NULL)
  {
    next = curr->next;
    curr->next = prev;
    prev = curr;
    curr = next;
  }
  t->employeelist.head = prev;
  t->employeelist.tail = old_head;

  save_table_binary(t);

  char log_details[64];
  snprintf(log_details, sizeof(log_details), "Reversed Table ID: %d", t->id);
  add_log(atoi(owner_id_str), "UPDATE", log_details);

  pthread_mutex_unlock(&t->lock);

  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{\"status\": \"Success\"}");
}

// --- Handle Recursive Reverse ---
void handle_recursive_reverse(struct mg_connection *c, struct mg_http_message *hm)
{
  char table_id_str[32], owner_id_str[32];

  // 1. Extract Both IDs
  if (mg_http_get_var(&hm->query, "table_id", table_id_str, sizeof(table_id_str)) <= 0 ||
      mg_http_get_var(&hm->query, "owner_id", owner_id_str, sizeof(owner_id_str)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing 'table_id' or 'owner_id'\" }");
    return;
  }

  // 2. Load Table
  Table *t = get_or_load_table(atoi(table_id_str), atoi(owner_id_str));
  if (!t)
  {
    mg_http_reply(c, 403, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Access Denied or Table Not Found\" }");
    return;
  }

  cJSON *json_array = cJSON_CreateArray();

  pthread_mutex_lock(&t->lock);
  recursive_json_builder(t->employeelist.head, json_array);
  pthread_mutex_unlock(&t->lock);

  char *response_str = cJSON_PrintUnformatted(json_array);
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", response_str);

  free(response_str);
  cJSON_Delete(json_array);
}

// --- Handle Update ---
void handle_update(struct mg_connection *c, struct mg_http_message *hm)
{
  // 1. Parse JSON Safely
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Invalid JSON\" }");
    return;
  }

  // 2. Extract & Validate Meta Fields
  cJSON *j_table_id = cJSON_GetObjectItem(json, "table_id");
  cJSON *j_owner_id = cJSON_GetObjectItem(json, "owner_id");
  cJSON *j_orig_id = cJSON_GetObjectItem(json, "original_id");
  cJSON *j_pos = cJSON_GetObjectItem(json, "position");
  cJSON *j_data = cJSON_GetObjectItem(json, "data");

  if (!j_table_id || !cJSON_IsNumber(j_table_id) ||
      !j_owner_id || !cJSON_IsNumber(j_owner_id) ||
      !j_orig_id || !j_data)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing required fields\" }");
    cJSON_Delete(json);
    return;
  }

  int original_id = j_orig_id->valueint;
  int target_pos = j_pos ? j_pos->valueint : -1;

  // 3. Extract & Validate Data Fields
  cJSON *j_id = cJSON_GetObjectItem(j_data, "id");
  cJSON *j_name = cJSON_GetObjectItem(j_data, "name");
  cJSON *j_age = cJSON_GetObjectItem(j_data, "age");
  cJSON *j_dept = cJSON_GetObjectItem(j_data, "department");
  cJSON *j_salary = cJSON_GetObjectItem(j_data, "salary");

  // Check Types
  if (!j_id || !cJSON_IsNumber(j_id) ||
      !j_name || !cJSON_IsString(j_name) ||
      !j_age || !cJSON_IsNumber(j_age) ||
      !j_dept || !cJSON_IsString(j_dept) ||
      !j_salary || !cJSON_IsNumber(j_salary))
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Data type mismatch\" }");
    cJSON_Delete(json);
    return;
  }

  // 4. FIX: Use get_or_load_table with Integers
  Table *t = get_or_load_table(j_table_id->valueint, j_owner_id->valueint);
  if (!t)
  {
    mg_http_reply(c, 403, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Access Denied or Table Not Found\" }");
    cJSON_Delete(json);
    return;
  }

  // Create node
  emp *new_node = create_node_from_json(j_data);
  if (!new_node)
  {
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Server Memory Error\" }");
    cJSON_Delete(json);
    return;
  }

  // --- CRITICAL SECTION STARTS ---
  pthread_mutex_lock(&t->lock);

  // 5. DATA LOGIC VALIDATION
  if (!isOnlyAlphaSpaces(new_node->name) || strlen(new_node->name) >= 50)
  {
    pthread_mutex_unlock(&t->lock);
    free(new_node);
    cJSON_Delete(json);
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"status\": \"Error\", \"message\": \"Invalid Name\" }");
    return;
  }
  if (!isOnlyAlphaSpaces(new_node->department))
  {
    pthread_mutex_unlock(&t->lock);
    free(new_node);
    cJSON_Delete(json);
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"status\": \"Error\", \"message\": \"Invalid Department\" }");
    return;
  }
  if (new_node->age < 16 || new_node->salary < 0)
  {
    pthread_mutex_unlock(&t->lock);
    free(new_node);
    cJSON_Delete(json);
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"status\": \"Error\", \"message\": \"Invalid Age or Salary\" }");
    return;
  }

  // 6. Collision Check (If changing ID)
  if (new_node->id != original_id)
  {
    emp *check = t->employeelist.head;
    while (check)
    {
      if (check->id == new_node->id)
      {
        pthread_mutex_unlock(&t->lock);
        free(new_node);
        cJSON_Delete(json);
        mg_http_reply(c, 409, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"New ID already exists\" }");
        return;
      }
      check = check->next;
    }
  }

  // 7. FIND & DETACH OLD NODE
  emp *curr = t->employeelist.head;
  emp *prev = NULL;
  int found = 0;
  while (curr != NULL)
  {
    if (curr->id == original_id)
    {
      if (prev == NULL)
        t->employeelist.head = curr->next;
      else
        prev->next = curr->next;

      if (curr == t->employeelist.tail)
        t->employeelist.tail = prev;

      free(curr);
      found = 1;
      break;
    }
    prev = curr;
    curr = curr->next;
  }

  if (!found)
  {
    pthread_mutex_unlock(&t->lock);
    free(new_node);
    cJSON_Delete(json);
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Original ID not found\" }");
    return;
  }

  // 8. INSERT NEW NODE
  insert_node_at_pos(t, new_node, target_pos);

  // 9. Save & Unlock
  save_table_binary(t);

  char log_details[128];
  snprintf(log_details, sizeof(log_details), "Updated Employee ID: %d", new_node->id);
  add_log(j_owner_id->valueint, "UPDATE", log_details);

  pthread_mutex_unlock(&t->lock);

  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"success\", \"message\": \"Employee Updated\" }");
  cJSON_Delete(json);
}
