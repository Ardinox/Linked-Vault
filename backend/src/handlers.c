// File_Name handler.c
// Serves the logic for operations

#include "handlers.h"
#include "employee.h"
#include "utils.h"
#include "table.h"
#include "storage.h"

void handle_get_tables(struct mg_connection *c, struct mg_http_message *hm){
  // Create JSON Array
    cJSON *json_array = cJSON_CreateArray();

    pthread_mutex_lock(&global_list_lock);

    Table *curr = global_tables_head;
    while (curr != NULL)
    {
        // Add the string ID to the array
        cJSON_AddItemToArray(json_array, cJSON_CreateString(curr->table_id));
        curr = curr->next;
    }
    // Send Response
    char *response_str = cJSON_PrintUnformatted(json_array);
    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", 
                  "%s", response_str);

    pthread_mutex_unlock(&global_list_lock);

    free(response_str);
    cJSON_Delete(json_array);
}

// --- 1. Handles insertion (Supports insertion at specific position) ---
void handle_insertion(struct mg_connection *c, struct mg_http_message *hm)
{
  // Parse the incoming HTTP body into a JSON object
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);

  // Safety Check for parsing
  if (!json)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Invalid JSON\" }");
    return;
  }

  // --- Get Table Id ---
  cJSON *j_table_id = cJSON_GetObjectItem(json, "table_id");
  if (!j_table_id || !cJSON_IsString(j_table_id))
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Missing Table ID\" }");
    cJSON_Delete(json);
    return;
  }

  // --- Get the Table Struct ---
  Table *t = get_or_create_table(j_table_id->valuestring);
  if (!t)
  {
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Table Creation Failed\" }");
    cJSON_Delete(json);
    return;
  }

  // Extract "position"
  // Default to -1 (End of list) if not provided.
  int position = -1;
  cJSON *j_pos = cJSON_GetObjectItem(json, "position");
  if (j_pos)
    position = j_pos->valueint;

  // Extract "data" object
  cJSON *j_data = cJSON_GetObjectItem(json, "data");

  // Create the struct in heap memory
  emp *insert = create_node_from_json(j_data);
  if (insert == NULL)
  {
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Server Out of Memory\" }");
    cJSON_Delete(json);
    return;
  }

  // lock using Mutex lock
  pthread_mutex_lock(&t->lock);

  // validate Inputs
  const char *error_msg = validate_employee_json(j_data, t);
  if (error_msg != NULL)
  {
    pthread_mutex_unlock(&t->lock);
    free(insert);

    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"%s\" }", error_msg);
    cJSON_Delete(json);
    return;
  }

  // Logic: Linked List Insertion
  // Case 1: List is Empty
  if (t->employeelist.head == NULL)
  {
    t->employeelist.head = insert;
    t->employeelist.tail = insert; // Head and Tail are the same node
  }

  // Case 2: Insert at Head
  else if (position == 0)
  {
    insert->next = t->employeelist.head;
    t->employeelist.head = insert;
  }

  // Case 3: Insert at Tail (Append)
  else if (position == -1)
  {
    t->employeelist.tail->next = insert;
    t->employeelist.tail = insert; // Point current tail to new node
  }

  // Case 4: Insert at Specific Index (Middle)
  else
  {
    emp *curr = t->employeelist.head;
    emp *prev = NULL;
    int curr_pos = 0;

    while (curr != NULL && curr_pos < position)
    {
      prev = curr;
      curr = curr->next;
      curr_pos++;
    }

    // If position > list_length, append to end
    if (curr == NULL)
    {
      t->employeelist.tail->next = insert;
      t->employeelist.tail = insert;
    }
    else
    {
      prev->next = insert;
      insert->next = curr;
    }
  }

  // Now that the list is modified, save it immediately
  save_table_binary(t);

  // Unlock after insertion completion
  pthread_mutex_unlock(&t->lock);

  // Sending the json response for successful insertion
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"success\", \"message\": \"Inserted at pos %d\", \"id\": %d }",
                position, insert->id);

  // Deleting the json to clear memory
  cJSON_Delete(json);
}

// --- Handles Display (returns all employees as a JSON Array) ---
void handle_showall(struct mg_connection *c, struct mg_http_message *hm)
{
  // Extract 'table_id' from the URL Query String
  char table_id[50];
  if (mg_http_get_var(&hm->query, "table_id", table_id, sizeof(table_id)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing 'table_id' parameter in URL\" }");
    return;
  }

  //Get the specific Table
  Table *t = get_or_create_table(table_id);
  if(!t)
  {
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Table Creation Failed\" }");
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
  char id_str[32], table_id[50];

  // Extract Id from query string
  if (mg_http_get_var(&hm->query, "id", id_str, sizeof(id_str)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing 'id' parameter in URL\" }");
    return;
  }
  // Extract Table Id from query string
  if (mg_http_get_var(&hm->query, "table_id", table_id, sizeof(table_id)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing 'table_id' parameter in URL\" }");
    return;
  }
  // Get the specific Table
  Table *t = get_or_create_table(table_id);
  if(!t)
  {
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Table Creation Failed\" }");
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
                  "{\"message\": \"The Id %d Doesn't exist\"}", target_id);
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
  char id_str[32], table_id[50];
  // Extract id from query string
  if (mg_http_get_var(&hm->query, "id", id_str, sizeof(id_str)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"'id' Field not found in the URL\" }");
    return;
  }

  // Extract id from query string
  if (mg_http_get_var(&hm->query, "table_id", table_id, sizeof(table_id)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"'table_id' Field not found in the URL\" }");
    return;
  }

  Table *t = get_or_create_table(table_id);
  if(!t){
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Table creation failed\" }");
    return;
  }

  int target_id = atoi(id_str);

  // lock the list
  pthread_mutex_lock(&t->lock);

  // Check if list is empty
  if (t->employeelist.head == NULL)
  {
    // unlock the list
    pthread_mutex_unlock(&t->lock);
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"List is Empty\" }");
    return;
  }

  emp *curr = t->employeelist.head;
  emp *prev = NULL;

  // Deleting the HEAD node
  if (curr != NULL && curr->id == target_id)
  {
    t->employeelist.head = curr->next;
    if (t->employeelist.head == NULL)
    {
      t->employeelist.tail = NULL;
    }

    save_table_binary(t);

    // unlock the list
    pthread_mutex_unlock(&t->lock);

    free(curr); // Free the memory of the deleted node

    // Response
    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"success\", \"message\": \"Deleted id %d\" }", target_id);
    return;
  }

  // General Case
  while (curr != NULL && curr->id != target_id)
  {
    prev = curr;
    curr = curr->next;
  }

  // Not Found
  if (curr == NULL)
  {
    // unlock the list
    pthread_mutex_unlock(&t->lock);
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{\"message\": \"ID %d not found\"}", target_id);
    return;
  }

  // Node Found: Unlink it
  prev->next = curr->next;
  if (curr == t->employeelist.tail)
  {
    t->employeelist.tail = prev;
  }

  save_table_binary(t);

  // unlock the list
  pthread_mutex_unlock(&t->lock);

  // Cleanup
  free(curr);

  // Response
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"success\", \"message\": \"Deleted id %d\" }", target_id);
}

// --- Reverse Linked List ---
void handle_reverse(struct mg_connection *c, struct mg_http_message *hm)
{
  char table_id[50];
  // Extract Table Id from query string
  if (mg_http_get_var(&hm->query, "table_id", table_id, sizeof(table_id)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing 'table_id' parameter in URL\" }");
    return;
  }

  Table *t = get_or_create_table(table_id);
  if(!t){
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Table creation failed\" }");
    return;
  }

  // lock the list
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

  // Save to Disk
  save_table_binary(t);

  // unlock the list
  pthread_mutex_unlock(&t->lock);
  // Response
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{\"status\": \"Success\"}");
}

// --- Handle Recursive Reverse ---
void handle_recursive_reverse(struct mg_connection *c, struct mg_http_message *hm)
{
  // Extract 'table_id' from the URL Query String
  char table_id[50];
  if (mg_http_get_var(&hm->query, "table_id", table_id, sizeof(table_id)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing 'table_id' parameter in URL\" }");
    return;
  }

  //Get the specific Table
  Table *t = get_or_create_table(table_id);
  if(!t)
  {
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Table Creation Failed\" }");
    return;
  }

  cJSON *json_array = cJSON_CreateArray();

  // lock the list
  pthread_mutex_lock(&t->lock);

  // Helper function located in utilis.c
  recursive_json_builder(t->employeelist.head, json_array);

  // unlock the list
  pthread_mutex_unlock(&t->lock);

  char *response_str = cJSON_PrintUnformatted(json_array);

  // Response
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", response_str);

  // Cleanup
  free(response_str);
  cJSON_Delete(json_array);
}

// --- Handle CSV Export ---
void handle_export(struct mg_connection *c, struct mg_http_message *hm)
{
  char table_id[50];
  // Extract id from query string
  if (mg_http_get_var(&hm->query, "table_id", table_id, sizeof(table_id)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"'table_id' Field not found in the URL\" }");
    return;
  }

  Table *t = get_or_create_table(table_id);
  if(!t){
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Table creation failed\" }");
    return;
  }

  // 'Content-Disposition: attachment' forces the browser to download the file.
  char header_buffer[512];
  snprintf(header_buffer, sizeof(header_buffer),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/csv\r\n"
    "Content-Disposition: attachment; filename=\"%s_data.csv\"\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "Connection: close\r\n"
    "\r\n",
    t->table_id);

  // Send Headers First
  mg_send(c, header_buffer, strlen(header_buffer));

  // Send CSV Column Headers
  char *csv_header_row = "ID,Name,Age,Department,Salary\n";
  mg_send(c, csv_header_row, strlen(csv_header_row));

  // lock the list
  pthread_mutex_lock(&t->lock);

  // Loop and Stream Data
  emp *curr = t->employeelist.head;
  char buffer[1024];

  while (curr != NULL)
  {
    // Format the current node into a CSV string
    int line_len = snprintf(buffer, sizeof(buffer), "%d,%s,%d,%s,%d\n", curr->id, curr->name, curr->age, curr->department, curr->salary);

    size_t len_to_send = 0;

    if (line_len < 0)
    {
      // Encoding error, skip this line
    }
    else if ((size_t)line_len >= sizeof(buffer))
    {
      // String was truncated.
      len_to_send = sizeof(buffer) - 1;
    }
    else
    {
      len_to_send = (size_t)line_len;
    }

    // Send this specific line to the client
    mg_send(c, buffer, len_to_send);
    curr = curr->next;
  }
  // unlock the list
  pthread_mutex_unlock(&t->lock);

  // 5. Signal Mongoose that we are done
  c->is_draining = 1;
}

// --- Handle CSV Import ---
void handle_import(struct mg_connection *c, struct mg_http_message *hm)
{
  char table_id[50];
  // Extract id from query string
  if (mg_http_get_var(&hm->query, "table_id", table_id, sizeof(table_id)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"'table_id' Field not found in the URL\" }");
    return;
  }

  Table *t = get_or_create_table(table_id);
  if(!t){
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Table creation failed\" }");
    return;
  }

  // Basic Safety Checks
  if (hm == NULL || hm->body.len == 0 || hm->body.buf == NULL)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Empty CSV Body\" }");
    return;
  }

  // Allocate Memory
  char *csv_content = malloc(hm->body.len + 1);
  if (!csv_content)
  {
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Server Memory Error\" }");
    return;
  }

  // Copy Data and Null Terminate
  memcpy(csv_content, hm->body.buf, hm->body.len);
  csv_content[hm->body.len] = '\0';

  // 4. LOCK ONCE
  pthread_mutex_lock(&t->lock);

  // Parse Lines (Safe Method)
  char *cursor = csv_content;
  char *line_start = cursor;
  int count = 0;
  int skipped = 0;

  while (*cursor != '\0')
  {
    // Find the end of the current line (newline or end of string)
    size_t len = strcspn(line_start, "\n");

    // Stop if we hit a purely empty end
    if (len == 0 && line_start[0] == '\0')
      break;

    // Temporarily terminate the line string
    char original_char = line_start[len];
    line_start[len] = '\0';

    // Remove carriage return '\r' (common in Windows CSVs)
    char *cr = strchr(line_start, '\r');
    if (cr)
    {
      *cr = '\0';
    }

    // Process non-empty lines
    if (strlen(line_start) > 5)
    {
      int id, age, salary;
      char name[50], dept[50];

      // Parse using corrected order and buffer protection
      // Note: append_to_list expects (id, name, age, dept, salary)
      if (sscanf(line_start, "%d,%49[^,],%d,%49[^,],%d",
                 &id, name, &age, dept, &salary) == 5)
      {
        // validation
        const char *error_msg = validate_core_logic(t, id, name, age, dept, salary);
        if (error_msg == NULL)
        {
          append_to_list(t, id, name, age, dept, salary);
          count++;
        }
        else
        {
          skipped++;
        }
      }
    }

    // Restore logic for next iteration
    if (original_char == '\0')
    {
      break; // End of file reached
    }
    line_start += len + 1; // Skip the newline character
    cursor = line_start;
  }

  // Save and Unlock
  save_table_binary(t);
  pthread_mutex_unlock(&t->lock);

  // Cleanup and Reply
  free(csv_content);

  mg_http_reply(c, 200,
                "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"Success\", \"added\": %d, \"skipped\": %d }", count, skipped);
}

// --- Handle Linkedlist cleanup ---
void handle_delete_linkedlist(struct mg_connection *c, struct mg_http_message *hm)
{
  char table_id[50];
  // Extract Table Id from query string
  if (mg_http_get_var(&hm->query, "table_id", table_id, sizeof(table_id)) <= 0)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Missing 'table_id' parameter in URL\" }");
    return;
  }

  Table *t = get_or_create_table(table_id);
  if(!t){
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{ \"status\": \"Error\", \"message\": \"Table creation failed\" }");
    return;
  }

  // lock the list
  pthread_mutex_lock(&t->lock);

  // Check if list is empty
  if (t->employeelist.head == NULL)
  {
    // unlock the list
    pthread_mutex_unlock(&t->lock);

    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"List is Empty\" }");
    return;
  }
  // Create a current and previous node
  emp *curr = t->employeelist.head;
  emp *prev = NULL;
  // loop runs untill
  while (curr != NULL)
  {
    prev = curr;
    curr = curr->next;
    // free up the previous node
    free(prev);
  }

  // Set the head and tail pointers to NULL
  t->employeelist.head = NULL;
  t->employeelist.tail = NULL;

  // Save the empty state to disk
  save_table_binary(t);

  // unlock the list
  pthread_mutex_unlock(&t->lock);

  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"Success\"}");
}