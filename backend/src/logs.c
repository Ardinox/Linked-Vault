// File_Name logs.c
#include "mongoose.h"
#include "cJSON.h"

#include "logs.h"

// Helper: Get current Timestamp
void get_time_string(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// 1. WRITE LOG (The "Add" Function)
void add_log(int owner_id, const char *action, const char *details) {
    // A. Ensure folder exists
    #ifdef _WIN32
        _mkdir("bin/logs");
    #else
        mkdir("bin/logs", 0777);
    #endif

    // B. Open User's specific log file
    char filename[64];
    snprintf(filename, sizeof(filename), "bin/logs/user_%d.log", owner_id);

    FILE *fp = fopen(filename, "a"); // "a" = Append
    if (!fp) return;

    char time_str[32];
    get_time_string(time_str, sizeof(time_str));

    // --- THE THREE THINGS YOU WANTED ---
    // 1. Timestamp | 2. Action | 3. Details
    fprintf(fp, "%s|%s|%s\n", time_str, action, details);
    
    fclose(fp);
    printf("[LOG] %s: %s\n", action, details);
}

