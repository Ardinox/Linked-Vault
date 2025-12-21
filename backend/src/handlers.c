// File_Name handler.c
// Serves the logic for operations

#include "cJSON.h"

#include "handlers.h"
#include "employee.h"
#include "utilis.h"

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

  // Extract "position"
  // Default to -1 (End of list) if not provided.
  int position = -1;
  cJSON *j_pos = cJSON_GetObjectItem(json, "position");
  if (j_pos)
    position = j_pos->valueint;

  // Extract "data" object
  cJSON *j_data = cJSON_GetObjectItem(json, "data");
  if (!j_data)
  {
    mg_http_reply(c, 400, "...", "{ \"error\": \"Missing 'data' object\" }");
    cJSON_Delete(json); // Cleanup
    return;
  }

  // ------Validations Checks------
  // Extract Fields
  cJSON *j_id = cJSON_GetObjectItem(j_data, "id");
  cJSON *j_name = cJSON_GetObjectItem(j_data, "name");
  cJSON *j_age = cJSON_GetObjectItem(j_data, "age");
  cJSON *j_dept = cJSON_GetObjectItem(j_data, "department");
  cJSON *j_salary = cJSON_GetObjectItem(j_data, "salary");

  // 1. Check Missing Fields
  if (!j_id || !j_name || !j_age || !j_dept || !j_salary)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"All fields are required\" }");
    cJSON_Delete(json);
    return;
  }

  // 2. Check Name Format (Prevent special chars/scripts)
  if (!isOnlyAlphaSpaces(j_name->valuestring))
  {
    mg_http_reply(c, 409, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Name field should only contain alphabet and spaces!\" }");
    cJSON_Delete(json);
    return;
  }

  // 3. Check Numbers are actually numbers
  if (!cJSON_IsNumber(j_id) || !cJSON_IsNumber(j_age) || !cJSON_IsNumber(j_salary))
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"ID, Age, and Salary must be numbers\" }");
    cJSON_Delete(json);
    return;
  }

  // 4. CHECK UNIQUE ID
  int new_id = j_id->valueint;
  emp *scanner = my_list.head;
  while (scanner != NULL)
  {
    if (scanner->id == new_id)
    {
      // ID FOUND! Return Error immediately.
      mg_http_reply(c, 409, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                    "{ \"status\": \"Error\", \"message\": \"Employee ID %d already exists!\" }", new_id);
      cJSON_Delete(json);
      return;
    }
    scanner = scanner->next;
  }

  // 5. Check for valid age
  if (j_age->valueint < 16 || j_age->valueint > 100)
  {
    mg_http_reply(c, 409, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Enter a valid Age!\" }");
    cJSON_Delete(json);
    return;
  }

  // 6. Check Department Format (Prevent special chars/scripts)
  if (!isOnlyAlphaSpaces(j_dept->valuestring))
  {
    mg_http_reply(c, 409, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Department field should only contain alphabet and spaces!\" }");
    cJSON_Delete(json);
    return;
  }

  // 7. Check for valid salary (salary can't be negative)
  if (j_salary->valueint < 0)
  {
    mg_http_reply(c, 409, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Enter a Valid Salary!\" }");
    cJSON_Delete(json);
    return;
  }

  // 8. Check for available charector size for Name (Buffer Overflow Prevention)
  if (strlen(j_name->valuestring) >= 50)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Name is too long! Max 49 characters allowed.\" }");
    cJSON_Delete(json);
    return;
  }

  // 9. Check for available character size for Department (Buffer Overflow Prevention)
  if (strlen(j_dept->valuestring) >= 50)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Department is too long! Max 49 characters allowed.\" }");
    cJSON_Delete(json);
    return;
  }

  // --- validation Completed ---

  // Create the struct in heap memory
  emp *insert = create_node_from_json(j_data);
  if (insert == NULL)
  {
    mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Server Out of Memory\" }");
    cJSON_Delete(json);
    return;
  }

  // Logic: Linked List Insertion

  // Case 1: List is Empty
  if (my_list.head == NULL)
  {
    my_list.head = insert;
    my_list.tail = insert; // Head and Tail are the same node
  }

  // Case 2: Insert at Head
  else if (position == 0)
  {
    insert->next = my_list.head;
    my_list.head = insert;
  }

  // Case 3: Insert at Tail (Append)
  else if (position == -1)
  {
    my_list.tail->next = insert;
    my_list.tail = insert; // Point current tail to new node
  }

  // Case 4: Insert at Specific Index (Middle)
  else
  {
    emp *curr = my_list.head;
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
      my_list.tail->next = insert;
      my_list.tail = insert;
    }
    else
    {
      prev->next = insert;
      insert->next = curr;
    }
  }

  // Sending the json response for successful insertion
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"success\", \"message\": \"Inserted at pos %d\", \"id\": %d }",
                position, insert->id);

  // Deleting the json to clear memory
  cJSON_Delete(json);
}

// --- Handles Display (returns all employees as a JSON Array) ---
void handle_showall(struct mg_connection *c)
{
  cJSON *json_array = cJSON_CreateArray();
  emp *curr = my_list.head;

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

  // Successful Response with data
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", response_str);

  // Cleanup
  free(response_str);
  cJSON_Delete(json_array);
}

// --- Handles Search by ID (Linear Search: as the data is not sorted according to ID) ---
void handle_search_by_id(struct mg_connection *c, struct mg_http_message *hm)
{
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Invalid JSON\" }");
    return;
  }

  cJSON *j_id = cJSON_GetObjectItem(json, "id");
  if (!j_id)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Provide id in JSON\" }");
    cJSON_Delete(json);
    return;
  }
  int target_id = j_id->valueint;

  // Linear Search for the ID
  emp *curr = my_list.head;
  while (curr != NULL && curr->id != target_id)
  {
    curr = curr->next;
  }

  // If curr is NULL, we reached the end without finding the ID
  if (curr == NULL)
  {
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{\"message\": \"The Id %d Doesn't exist\"}", target_id);
    cJSON_Delete(json);
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

  // Response
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", response_str);

  //Cleanup
  free(response_str);
  cJSON_Delete(emp_obj);
  cJSON_Delete(json);
}

// --- Handles Deletion (Removes a node by ID and frees its memory) ---
void handle_delete(struct mg_connection *c, struct mg_http_message *hm)
{
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Invalid JSON\" }");
    return;
  }

  cJSON *j_id = cJSON_GetObjectItem(json, "id");
  if (!j_id)
  {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"Provide id in JSON\" }");
    cJSON_Delete(json);
    return;
  }
  int target_id = j_id->valueint;

  // Check if list is empty
  if (my_list.head == NULL)
  {
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"Error\", \"message\": \"List is Empty\" }");
    cJSON_Delete(json);
    return;
  }

  emp *curr = my_list.head;
  emp *prev = NULL;

  // Deleting the HEAD node
  if (curr != NULL && curr->id == target_id)
  {
    my_list.head = curr->next;
    if (my_list.head == NULL)
      my_list.tail = NULL;
    free(curr); // Free the memory of the deleted node

    // Response
    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{ \"status\": \"success\", \"message\": \"Deleted id %d\" }", target_id);

    // Cleanup
    cJSON_Delete(json);
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
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                  "{\"message\": \"ID %d not found\"}", target_id);
    cJSON_Delete(json);
    return;
  }

  // Node Found: Unlink it
  prev->next = curr->next;
  if (curr == my_list.tail)
    my_list.tail = prev;

  // Cleanup
  free(curr);

  // Response
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"success\", \"message\": \"Deleted id %d\" }", target_id);
  cJSON_Delete(json);
}

// --- Reverse Linked List ---
void handle_reverse(struct mg_connection *c, struct mg_http_message *hm)
{
  emp *prev = NULL, *curr = my_list.head, *next = NULL;
  emp *old_head = my_list.head;
  while (curr != NULL)
  {
    next = curr->next;
    curr->next = prev;
    prev = curr;
    curr = next;
  }
  my_list.head = prev;
  my_list.tail = old_head;

  // Response
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{\"status\": \"Success\"}");
}

// --- Handle Recursive Reverse ---
void handle_recursive_reverse(struct mg_connection *c)
{
  cJSON *json_array = cJSON_CreateArray();

  // Helper function located in utilis.c
  recursive_json_builder(my_list.head, json_array);

  char *responce_str = cJSON_PrintUnformatted(json_array);

  // Response
  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", responce_str);

  // Cleanup
  free(responce_str);
  cJSON_Delete(json_array);
}
