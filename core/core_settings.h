#pragma once

#define i_type vec_config_path_t // NOLINT
#define i_key  char *            // NOLINT
#include "stc/vec.h"

struct core_settings_s
{

    char *log_path;

    char *core_log_file;
    char *core_log_level;
    bool  core_log_console;

    char *network_log_file;
    char *network_log_level;
    bool  network_log_console;

    char *dns_log_file;
    char *dns_log_level;
    bool  dns_log_console;

    int   workers_count;
    char *libs_path;

    vec_config_path_t config_paths;
};

void parseCoreSettings(char *data_json);
void increaseFileLimit();

char *getCoreLoggerFullPath();
char *getNetworkLoggerFullPath();
char *getDnsLoggerFullPath();

struct core_settings_s *getCoreSettings();
void                    initCoreSettings();
