// File_Name main.c
// Entry Point for the LinkedVault Backend Server

#include "handlers.h"
#include "employee.h"
#include "utils.h"
#include "storage.h"

// --- CONSTANTS ---
// Listening on 0.0.0.0 allows access from external IPs, not just localhost.
static const char *LISTENING_ADDR = "http://0.0.0.0:8000";

// The poll interval (1000ms) determines how long the call blocks if there are no events.
static const int POLL_INTERVAL_MS = 1000;

// Protects users.bin and tables_registry.bin operations
pthread_mutex_t file_registry_lock = PTHREAD_MUTEX_INITIALIZER;

// ---  EVENT HANDLER ---
static void fn(struct mg_connection *c, int ev, void *ev_data)
{
  if (ev == MG_EV_HTTP_MSG)
  {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    /* * --- CORS PREFLIGHT HANDLER ---
     * Browsers send an OPTIONS request before POST requests to check permissions.
     * We must reply with Access-Control-Allow-* headers, or the browser will block
     * the subsequent POST request.
     */
    if (is_method(hm, "OPTIONS"))
    {
      mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\n"
                            "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                            "Access-Control-Allow-Headers: Content-Type\r\n",
                    "");
      return;
    }

    //  --- ROUTING LOGIC ---
    if (mg_match(hm->uri, mg_str("/auth/get_user"), NULL) && is_method(hm, "POST"))
    {
      handle_get_user(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/auth/register"), NULL) && is_method(hm, "POST"))
    {
      handle_registry(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/meta/create_table"), NULL) && is_method(hm, "POST"))
    {
      handle_create_table(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/meta/list_tables"), NULL) && is_method(hm, "POST"))
    {
      handle_list_table(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/history"), NULL) && is_method(hm, "GET"))
    {
      handle_history(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/insert"), NULL) && is_method(hm, "POST"))
    {
      handle_insertion(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/show"), NULL) && is_method(hm, "GET"))
    {
      handle_showall(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/search"), NULL) && is_method(hm, "GET"))
    {
      handle_search_by_id(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/delete"), NULL) && is_method(hm, "DELETE"))
    {
      handle_delete(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/linkedreverse"), NULL) && is_method(hm, "PUT"))
    {
      handle_reverse(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/recursivereverse"), NULL) && is_method(hm, "GET"))
    {
      handle_recursive_reverse(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/update"), NULL) && is_method(hm, "PUT"))
    {
      handle_update(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/upload_csv"), NULL) && is_method(hm, "POST"))
    {
      handle_import(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/download_table"), NULL) && is_method(hm, "GET"))
    {
      handle_export(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/delete_table"), NULL) && is_method(hm, "DELETE"))
    {
      handle_clear_table(c, hm);
    }
    // Default Catch-All: 404 Not Found
    else
    {
      mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\n",
                  "Endpoint not found\n");
    }
  }
}

int main(void)
{
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);

  // Setup the HTTP listener.
  mg_http_listen(&mgr, LISTENING_ADDR, fn, NULL);
  printf("Server running on port 8000...\n");

  // --- Main Loop ---
  // This Infinite loop keep the server running
  for (;;)
  {
    mg_mgr_poll(&mgr, POLL_INTERVAL_MS);
  }

  // Cleanup (Good Practice)
  mg_mgr_free(&mgr);
  return 0;
}