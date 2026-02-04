// File_Name auth.handler.c
// Serves the data storing logic for auth operations

#include "handlers.h"
#include "employee.h"
#include "utils.h"
#include "table.h"
#include "storage.h"
#include "logs.h"

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

//  3. READS THE LOG FILES FOR INDIVIDUAL USERS
void handle_history(struct mg_connection *c, struct mg_http_message *hm)
{
  char owner_id_str[32];
  if (mg_http_get_var(&hm->query, "owner_id", owner_id_str, sizeof(owner_id_str)) <= 0)
  {
    mg_http_reply(c, 400, "", "{\"error\":\"Missing owner_id\"}");
    return;
  }

  int owner_id = atoi(owner_id_str);
  char filename[64];
  snprintf(filename, sizeof(filename), "bin/logs/user_%d.log", owner_id);

  FILE *fp = fopen(filename, "r");
  if (!fp)
  {
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "[]");
    return;
  }

  cJSON *root = cJSON_CreateArray();
  char line[512];

  // Read log file line by line
  while (fgets(line, sizeof(line), fp))
  {
    line[strcspn(line, "\n")] = 0; // Strip newline

    // Split by the pipe character '|'
    char *timestamp = strtok(line, "|");
    char *action = strtok(NULL, "|");
    char *details = strtok(NULL, "|");

    if (timestamp && action && details)
    {
      cJSON *item = cJSON_CreateObject();
      cJSON_AddStringToObject(item, "timestamp", timestamp);
      cJSON_AddStringToObject(item, "action", action);
      cJSON_AddStringToObject(item, "details", details); // <--- The details!
      cJSON_AddItemToArray(root, item);
    }
  }
  fclose(fp);

  char *json_str = cJSON_PrintUnformatted(root);
  mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json_str);
  free(json_str);
  cJSON_Delete(root);
}