#pragma once
// Send debug messages to serial port
// #define ENABLE_SERIAL_DEBUG

#ifdef ENABLE_SERIAL_DEBUG
#define DEBUGstart(spd)                                         \
    do                                                          \
    {                                                           \
        Serial.begin(spd);                                      \
        while (!Serial)                                         \
        {                                                       \
            digitalWrite(LED_BUILTIN, (millis() / 500) % 2);    \
        }                                                       \
    } while (0);                                                \
#define DEBUGt(...) Serial.print(__VA_ARGS__)
#define DEBUGln(...) Serial.println(__VA_ARGS__)
#define DEBUGf(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUGstart(...)
#define DEBUGt(...)
#define DEBUGln(...)
#define DEBUGf(...)
#endif