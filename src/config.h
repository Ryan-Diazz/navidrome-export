#ifndef CONFIG_H
#define CONFIG_H

#include "utilities.h"

Config loadConfig();

void cleanupConfig(Config& config);

void initializeConfig();

void resetConfig();

#endif