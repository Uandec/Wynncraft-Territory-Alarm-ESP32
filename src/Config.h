#pragma once

// Send debug messages to serial port
// #define ENABLE_SERIAL_DEBUG
// If the above line is uncommented, the program does not run until a serial connection is established.

// Wifi configuration
const char *SSID = "SSID";       // SSID of your WiFi
const char *PASSWORD = "PASSWORD"; // Password of your WiFi

// HTTPS Request frequency
const char *GUILD_PREFIX = "NONE";       // Prefix of the guild to keep track of (It is case sensitive)
#define UPDATE_FREQUENCY 10000          // How ofther to check with the Wynncraft API (min 6000 (ms))
