// File_Name table.handler.c
// Serves the data storing logic for table operations

#include "handlers.h"
#include "employee.h"
#include "utils.h"
#include "table.h"
#include "storage.h"
#include "logs.h"

// --- TABLE REGISTRY FUNCTIONS ---
// 3. CREATE TABLE
void handle_create_table(struct mg_connection *c, struct mg_http_message *hm)
{
  cJSON *json = cJSON_ParseWithLength(hm->body.buf, hm->body.len);
  if (!json)
    return;

  cJSON *owner_item = cJSON_GetObjectItem(json, "owner_id");
  cJSON *name_item = cJSON_GetObjectItem(json, "name");

  if (cJSON_IsNumber(owner_item) && cJSON_IsString(name_item))
  {
    int internal_id = save_table_metadata(owner_item->valueint, name_item->valuestring);

    unload_table(internal_id);

    char log_details[128];
    snprintf(log_details, sizeof(log_details), "Created Table: %s", name_item->valuestring);
    add_log(owner_item->valueint, "CREATE", log_details);

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

// --- Handle Table cleanup ---
void handle_clear_table(struct mg_connection *c, struct mg_http_message *hm)
{
  char table_id_str[32], owner_id_str[32];
  if (mg_http_get_var(&hm->query, "table_id", table_id_str, sizeof(table_id_str)) <= 0 ||
      mg_http_get_var(&hm->query, "owner_id", owner_id_str, sizeof(owner_id_str)) <= 0)
  {
    mg_http_reply(c, 400, "", "{\"error\":\"Missing params\"}");
    return;
  }

  int table_id = atoi(table_id_str);
  int owner_id = atoi(owner_id_str);

  // Call the safe storage function
  int res = delete_table_permanently(table_id, owner_id);

  if (res == 0)
  {
    char log_details[64];
    snprintf(log_details, sizeof(log_details), "Permanently Deleted Table ID: %d", table_id);
    add_log(owner_id, "DELETE-TABLE", log_details);

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "{\"status\":\"success\"}");
  }
  else
  {
    mg_http_reply(c, 404, "Content-Type: application/json\r\n",
                  "{\"error\":\"Table not found or permission denied\"}");
  }
}