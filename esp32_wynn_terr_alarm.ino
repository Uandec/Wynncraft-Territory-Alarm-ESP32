#include <WiFi.h>
#include <HTTPClient.h>
#include <TickTwo.h>

#include "src/Config.h"
#include "src/GlobalDebugSetup.h"
#include "src/TerritoryManager.hpp"
#include "src/UI.hpp"

#if UPDATE_FREQUENCY < 6000
#error Update frequency must be at least 6 seconds
#endif

#define WAKE_BUTTON_PIN 32

// Internal / Forward declarations
bool DisplayWakeButtonPressed = false;
void IRAM_ATTR isr() { Ui.ForceEnableDisplay(); }
void UpdateTerritories();
void ConnectToWifi();
// Timers
TickTwo httpsUpdate(UpdateTerritories, UPDATE_FREQUENCY);
TickTwo displayUpdate([]()
                      { Ui.UpdateDisplay(); },
                      DISPLAY_UPDATE_INTERVAL);

void setup()
{
  DEBUGstart(115200);
  Ui.Setup();
  WiFi.begin(SSID, PASSWORD);
  ConnectToWifi();
  UpdateTerritories();
  pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(WAKE_BUTTON_PIN, isr, FALLING);
  httpsUpdate.start();
  displayUpdate.start();
  Ui.SetupDone();
}

void loop()
{
  httpsUpdate.update();
  displayUpdate.update();
}

// Update tracked territories
void UpdateTerritories()
{
  // Store the start execution time
  unsigned long start = millis();
  DEBUGln(F("HTTP Requesting..."));

  if (WiFi.status() != WL_CONNECTED)
    ConnectToWifi();

  HTTPClient http;
  http.begin("https://api.wynncraft.com/v3/guild/list/territory");
  int httpResponseCode = http.GET();
  delay(0);
  if (httpResponseCode == HTTP_CODE_OK)
  {
    Stream &payload = http.getStream();
    TerritoryManager.UpdateTrackedTerrList(payload, GUILD_PREFIX);
    payload.flush();
    Ui.SetTerrTracked(TerritoryManager.GetTracedTerrCount());
    uint32_t territoriesLostSize = 0;
    Territory *territoriesLost = TerritoryManager.GetTrackedTerrLost(territoriesLostSize);
    Ui.SetTerrLostList(territoriesLost, territoriesLostSize);
    DEBUGt(F("Territories lost: "));
    DEBUGln(territoriesLostSize);
    for (uint32_t i = 0; i < territoriesLostSize; i++)
      DEBUGf("Territory: %s\t[Lost to: %s]\n", territoriesLost[i].name.c_str(), territoriesLost[i].guildPrefix);
  }
  else
  {
    DEBUGt(F("Wynncraft Error: "));
    DEBUGln(httpResponseCode);
        Ui.DisplayFailedMessage("HTTP Request Failed", 0);
  }
  http.end();
  // Print the execution time and available heap
  DEBUGf("Execution took: %d ms\n", millis() - start);
  DEBUGf("Available heap: %d\n", ESP.getFreeHeap());
}

// Helper function to manage connecting to WIFI
void ConnectToWifi()
{
  uint16_t attempts = 0;
  DEBUGln();
  DEBUGt(F("Connecting to WiFi..."));
  if (WiFi.status() == WL_CONNECTED)
  {
    DEBUGln(F("Already connected"));
    return;
  }
  delay(1000);

  while (WiFi.status() != WL_CONNECTED)
  {
    if (attempts >= 10)
    {
      DEBUGln();
      DEBUGln(F("Failed to connect to WiFi"));
      Ui.DisplayFailedMessage("Failed to connect to WiFi. Will retry again in 5 mins.", 60000);
      esp_sleep_enable_timer_wakeup(5 * 60 * 1000000); // light sleep for 5 mins
      esp_light_sleep_start();
      attempts = 0;
      DEBUGln();
      DEBUGt(F("Connecting to WiFi..."));
    }

    DEBUGt(F("."));
    attempts++;
    delay(1000);
  }
  DEBUGln();
  DEBUGln(F("Connected to WiFi"));
}
