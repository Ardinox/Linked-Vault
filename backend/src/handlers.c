// File_Name handler.c
// Serves the logic for operations

#include "handlers.h"
#include "employee.h"
#include "utils.h"
#include "table.h"
#include "storage.h"

// --- AUTH FUNCTIONS ---
// 1. GET USER (Login)
void handle_get_user(struct mg_connection *c, struct mg_http_message *hm)
{
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Invalid JSON\" }");
    return;
  }
  cJSON *username_item = cJSON_GetObjectItem(json, "username");

  if (username_item && cJSON_IsString(username_item))
  {
    User u = find_user_by_name(username_item->valuestring);

    if (u.id != -1)
    {
      // Found: Return ID and Hash
      mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                    "{ \"id\": %d, \"hash\": \"%s\" }", u.id, u.password_hash);
    }
    else
    {
      mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                    "{ \"status\": \"Error\", \"message\": \"User not found\" }");
    }
  }
  else
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing Credentials\" }");
  }
  cJSON_Delete(json);
}

// 2. REGISTER USER
void handle_registry(struct mg_connection *c, struct mg_http_message *hm)
{
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Invalid JSON\" }");
    return;
  }

  cJSON *user_item = cJSON_GetObjectItem(json, "username");
  cJSON *hash_item = cJSON_GetObjectItem(json, "hash");

  if (user_item && hash_item)
  {
    int new_id = save_new_user(user_item->valuestring, hash_item->valuestring);

    if (new_id > 0)
    {
      mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                    "{ \"status\": \"success\", \"id\": %d }", new_id);
    }
    else
    {
      mg_http_reply(c, 409, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                    "{ \"status\": \"Error\", \"message\": \"Username exists or DB error\" }");
    }
  }
  else
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing username or hash\" }");
  }
  cJSON_Delete(json);
}

// --- TABLE REGISTRY FUNCTIONS ---

// 3. CREATE TABLE
void handle_create_table(struct mg_connection *c, struct mg_http_message *hm)
{
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json)
    return;

  cJSON *owner_item = cJSON_GetObjectItem(json, "owner_id");
  cJSON *name_item = cJSON_GetObjectItem(json, "name");

  if (owner_item && name_item)
  {
    int internal_id = save_table_metadata(owner_item->valueint, name_item->valuestring);

    // Return the NEW Internal ID
    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"success\", \"internal_id\": %d }", internal_id);
  }
  else
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing owner_id or name\" }");
  }
  cJSON_Delete(json);
}

// 4. LIST TABLES
void handle_list_table(struct mg_connection *c, struct mg_http_message *hm)
{
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json)
    return;

  cJSON *owner_item = cJSON_GetObjectItem(json, "owner_id");

  if (owner_item)
  {
    int count = 0;
    TableMetadata *tables = get_user_tables(owner_item->valueint, &count);
    cJSON *arr = cJSON_CreateArray();

    if (tables)
    {
      for (int i = 0; i < count; i++)
      {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "name", tables[i].table_name);
        cJSON_AddNumberToObject(obj, "internal_id", tables[i].id); // Send 'id' not 'internal_id'
        cJSON_AddItemToArray(arr, obj);
      }
      free(tables);
    }
    char *response_str = cJSON_PrintUnformatted(arr);
    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", response_str);
    free(response_str);
    cJSON_Delete(arr);
  }
  else
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing owner_id\" }");
  }
  cJSON_Delete(json);
}

// --- 1. Handles insertion (Supports insertion at specific position) ---
void handle_insertion(struct mg_connection *c, struct mg_http_message *hm)
{
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json) {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"error\": \"Invalid JSON\" }");
    return;
  }

  cJSON *j_table_id = cJSON_GetObjectItem(json, "table_id");
  cJSON *j_owner_id = cJSON_GetObjectItem(json, "owner_id");

  if (!j_table_id || !cJSON_IsNumber(j_table_id) || !j_owner_id || !cJSON_IsNumber(j_owner_id)) {

    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"error\": \"Missing/Invalid table_id or owner_id\" }");
    cJSON_Delete(json);
    return;
  }


  Table *t = get_or_load_table(j_table_id->valueint, j_owner_id->valueint);
  if (!t) {
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
  if (j_pos) position = j_pos->valueint;

  insert_node_at_pos(t, insert, position);

  save_table_binary(t);
  
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
  char id_str[32], table_id_str[32], owner_id_str[32];

  if (mg_http_get_var(&hm->query, "id", id_str, sizeof(id_str)) <= 0 ||
      mg_http_get_var(&hm->query, "table_id", table_id_str, sizeof(table_id_str)) <= 0 ||
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

  int target_id = atoi(id_str);

  // Linear Search for the ID
  emp *curr = t->employeelist.head;
  while (curr != NULL && curr->id != target_id)
  {
    curr = curr->next;
  }

  // If curr is NULL, we reached the end without finding the ID
  if (curr == NULL)
  {
    // unlock the list
    pthread_mutex_unlock(&t->lock);

    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{\"message\": \"Id Not Found\"}");
    return;
  }

  // Found it: Build Response
  cJSON *emp_obj = cJSON_CreateObject();
  cJSON_AddNumberToObject(emp_obj, "id", curr->id);
  cJSON_AddStringToObject(emp_obj, "name", curr->name);
  cJSON_AddNumberToObject(emp_obj, "age", curr->age);
  cJSON_AddStringToObject(emp_obj, "department", curr->department);
  cJSON_AddNumberToObject(emp_obj, "salary", curr->salary);

  char *response_str = cJSON_PrintUnformatted(emp_obj);

  // unlock the list
  pthread_mutex_unlock(&t->lock);

  // Response
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", response_str);

  // Cleanup
  free(response_str);
  cJSON_Delete(emp_obj);
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
        pthread_mutex_unlock(&t->lock); free(new_node); cJSON_Delete(json);
        mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"status\": \"Error\", \"message\": \"Invalid Name\" }");
        return;
    }
    if (!isOnlyAlphaSpaces(new_node->department))
    {
        pthread_mutex_unlock(&t->lock); free(new_node); cJSON_Delete(json);
        mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{ \"status\": \"Error\", \"message\": \"Invalid Department\" }");
        return;
    }
    if (new_node->age < 16 || new_node->salary < 0)
    {
        pthread_mutex_unlock(&t->lock); free(new_node); cJSON_Delete(json);
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
            if (prev == NULL) t->employeelist.head = curr->next;
            else prev->next = curr->next;

            if (curr == t->employeelist.tail) t->employeelist.tail = prev;

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
    pthread_mutex_unlock(&t->lock);

    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"success\", \"message\": \"Employee Updated\" }");
    cJSON_Delete(json);
}

// --- Handle CSV Export ---
void handle_export(struct mg_connection *c, struct mg_http_message *hm)
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

  // 3. Prepare Headers
  char header_buffer[512];
  snprintf(header_buffer, sizeof(header_buffer),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/csv\r\n"
           "Content-Disposition: attachment; filename=\"%d_data.csv\"\r\n" // Use %d for int ID
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n"
           "\r\n",
           t->id);

  mg_send(c, header_buffer, strlen(header_buffer));
  
  char *csv_header_row = "ID,Name,Age,Department,Salary\n";
  mg_send(c, csv_header_row, strlen(csv_header_row));

  // 4. Stream Data
  pthread_mutex_lock(&t->lock);

  emp *curr = t->employeelist.head;
  char buffer[1024];

  while (curr != NULL)
  {
    int line_len = snprintf(buffer, sizeof(buffer), "%d,%s,%d,%s,%d\n", 
                            curr->id, curr->name, curr->age, curr->department, curr->salary);
    
    // Safety check for truncation
    if (line_len > 0 && (size_t)line_len < sizeof(buffer)) {
        mg_send(c, buffer, line_len);
    }
    curr = curr->next;
  }
  
  pthread_mutex_unlock(&t->lock);
  c->is_draining = 1; // Close connection after download
}

// --- Handle CSV Import ---
void handle_import(struct mg_connection *c, struct mg_http_message *hm)
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

  // 3. Safety Checks
  if (hm->body.len == 0 || hm->body.buf == NULL)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Empty CSV Body\" }");
    return;
  }

  char *csv_content = malloc(hm->body.len + 1);
  if (!csv_content)
  {
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Server Memory Error\" }");
    return;
  }

  memcpy(csv_content, hm->body.buf, hm->body.len);
  csv_content[hm->body.len] = '\0';

  pthread_mutex_lock(&t->lock);

  // 4. Parse Logic
  char *cursor = csv_content;
  char *line_start = cursor;
  int count = 0;
  int skipped = 0;

  while (*cursor != '\0')
  {
    size_t len = strcspn(line_start, "\n");
    if (len == 0 && line_start[0] == '\0') break;

    char original_char = line_start[len];
    line_start[len] = '\0';

    char *cr = strchr(line_start, '\r');
    if (cr) *cr = '\0';

    if (strlen(line_start) > 5)
    {
      int id, age, salary;
      char name[50], dept[50];

      if (sscanf(line_start, "%d,%49[^,],%d,%49[^,],%d", &id, name, &age, dept, &salary) == 5)
      {
        const char *error_msg = validate_core_logic(t, id, name, age, dept, salary);
        if (error_msg == NULL)
        {
          append_to_list(t, id, name, age, dept, salary);
          count++;
        }
        else skipped++;
      }
    }

    if (original_char == '\0') break;
    line_start += len + 1;
    cursor = line_start;
  }

  save_table_binary(t);
  pthread_mutex_unlock(&t->lock);

  free(csv_content);

  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"Success\", \"added\": %d, \"skipped\": %d }", count, skipped);
}

// --- Handle Linkedlist cleanup ---
void handle_delete_linkedlist(struct mg_connection *c, struct mg_http_message *hm)
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

  pthread_mutex_lock(&t->lock);

  if (t->employeelist.head == NULL)
  {
    pthread_mutex_unlock(&t->lock);
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"List is already empty\" }");
    return;
  }

  emp *curr = t->employeelist.head;
  while (curr != NULL)
  {
    emp *next = curr->next;
    free(curr);
    curr = next;
  }

  t->employeelist.head = NULL;
  t->employeelist.tail = NULL;

  save_table_binary(t);
  pthread_mutex_unlock(&t->lock);

  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"Success\"}");
}