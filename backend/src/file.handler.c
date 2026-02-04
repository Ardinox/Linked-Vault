// File_Name file.handler.c
// Serves the File export/import logic for table datas

#include "handlers.h"
#include "employee.h"
#include "utils.h"
#include "table.h"
#include "storage.h"
#include "logs.h"

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

  char log_details[64];
  snprintf(log_details, sizeof(log_details), "Downloaded Table ID: %d", t->id);
  add_log(atoi(owner_id_str), "EXPORT", log_details);

  emp *curr = t->employeelist.head;
  char buffer[1024];

  while (curr != NULL)
  {
    int line_len = snprintf(buffer, sizeof(buffer), "%d,%s,%d,%s,%d\n",
                            curr->id, curr->name, curr->age, curr->department, curr->salary);

    // Safety check for truncation
    if (line_len > 0 && (size_t)line_len < sizeof(buffer))
    {
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
    if (len == 0 && line_start[0] == '\0')
      break;

    char original_char = line_start[len];
    line_start[len] = '\0';

    char *cr = strchr(line_start, '\r');
    if (cr)
      *cr = '\0';

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
        else
          skipped++;
      }
    }

    if (original_char == '\0')
      break;
    line_start += len + 1;
    cursor = line_start;
  }

  save_table_binary(t);

  char log_details[128];
  snprintf(log_details, sizeof(log_details), "Imported CSV: Added %d, Skipped %d", count, skipped);
  add_log(atoi(owner_id_str), "IMPORT", log_details);

  pthread_mutex_unlock(&t->lock);

  free(csv_content);

  mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n",
                "{ \"status\": \"Success\", \"added\": %d, \"skipped\": %d }", count, skipped);
}

