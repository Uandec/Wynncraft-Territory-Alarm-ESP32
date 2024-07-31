#pragma once
#include <Arduino_GFX_Library.h>
#include <SPI.h>

#include "Config.h"
#include "Territory.hpp"
#include "GlobalDebugSetup.h"

// Display configuration
#define BUZZER_PIN 33
#define DISPLAY_UPDATE_INTERVAL 1000    // ms
#define DISPLAY_SLEEP_TIMEOUT 10        // seconds

class UiManager
{
public:
    UiManager(){};
    ~UiManager(){};

    void Setup();
    void SetupDone();
    void DisplayFailedMessage(const char *, uint32_t);

    void UpdateDisplay();
    void IRAM_ATTR ForceEnableDisplay();
    void SetTerrLostList(Territory *, uint32_t);
    void SetTerrTracked(uint32_t size) { m_TerritoriesTrackedSize = size; }

private:
    void BuzzTheBuzzer(uint32_t, uint32_t, uint32_t);
    void ResetDisplay();

    uint32_t m_TerritoriesTrackedSize = 0;
    uint32_t m_TerritoriesLostSize = 0;
    Territory *m_TerritoriesLostArray = nullptr;
    unsigned long m_LastUpdate = 0;

    int m_maxLetters = 0;

    bool m_DisplayOn = false;
    bool m_ForceUpdate = false;
    unsigned long m_ScreenOnTime = 0;
    Arduino_DataBus *m_bus = new Arduino_ESP32SPI(14, 5, 18, 23, GFX_NOT_DEFINED, VSPI); // DC, CS, SCK, MOSI, MISO, spi_num
    Arduino_GFX *m_gfx = new Arduino_SSD1331(m_bus, 12, 0);                              // RST, rotation

} Ui;

void UiManager::Setup()
{
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // Init Display
    if (!m_gfx->begin())
        DEBUGln("gfx->begin() failed!");

    m_gfx->fillScreen(BLACK);
    m_gfx->setCursor(0, 0);
    m_gfx->setTextColor(CYAN);
    m_gfx->println("Welcome!");
    m_gfx->setTextColor(WHITE);
    m_gfx->printf("Guild: ");
    m_gfx->setTextColor(DARKGREY);
    m_gfx->printf("%s\n", GUILD_PREFIX);
    m_gfx->setTextColor(ORANGE);
    m_gfx->println("Initializing....");

    m_maxLetters = floor(static_cast<double>(m_gfx->width()) / 6);
}

void UiManager::SetupDone()
{
    m_gfx->setTextColor(GREEN);
    m_gfx->println("DONE!");
    delay(1000);
    ResetDisplay();
}

void UiManager::DisplayFailedMessage(const char *message, uint32_t displayTime = 60000)
{
    ResetDisplay();
    m_gfx->setTextColor(RED);
    m_gfx->println(message);
    BuzzTheBuzzer(2, 1500, 4000);
    delay(displayTime);
    ResetDisplay();
}

void UiManager::UpdateDisplay()
{
    if (m_TerritoriesLostSize > 0)
    {
        if (!m_DisplayOn)
        {
            m_DisplayOn = true;
            m_gfx->fillScreen(BLACK);
        }
        m_ScreenOnTime = millis();
    }

    if (m_DisplayOn)
    {
        if (millis() - m_ScreenOnTime > DISPLAY_SLEEP_TIMEOUT * 1000)
        {
            ResetDisplay();
            return;
        }
        if (m_ForceUpdate)
        {
            m_ForceUpdate = false;
            m_gfx->fillScreen(BLACK);
        }

        m_gfx->setCursor(0, 0);
        m_gfx->setTextColor(WHITE);
        m_gfx->printf("Guild: ");
        m_gfx->setTextColor(DARKGREY);
        m_gfx->printf("%s\n", GUILD_PREFIX);
        m_gfx->setTextColor(WHITE);
        m_gfx->printf("Terrs: ");
        m_gfx->setTextColor(DARKGREY);
        m_gfx->printf("%d\n", m_TerritoriesTrackedSize);

        if (m_TerritoriesLostSize > 0)
        {
            m_gfx->setTextColor(WHITE);
            m_gfx->printf("Lost : ");
            m_gfx->setTextColor(RED);
            m_gfx->printf("%d\n", m_TerritoriesLostSize);
            if (m_TerritoriesLostSize == 1)
            {
                m_gfx->drawLine(0, 24, m_gfx->width(), 24, DARKGREY);
                m_gfx->setCursor(0, 26);
                m_gfx->setTextColor(WHITE);
                m_gfx->printf("   Territory   \n");
                m_gfx->setTextColor(DARKGREY);
                m_gfx->printf(m_TerritoriesLostArray[0].name.c_str());
                m_gfx->setTextColor(WHITE);
                m_gfx->printf("\nLost to: ");
                m_gfx->setTextColor(DARKGREY);
                m_gfx->printf("%s\n", m_TerritoriesLostArray[0].guildPrefix);
            }
        }
    }
}

void UiManager::SetTerrLostList(Territory *territories, uint32_t size)
{
    if (m_TerritoriesLostSize != 0)
    {
        m_TerritoriesLostSize = 0;
        delete[] m_TerritoriesLostArray;
        m_TerritoriesLostArray = nullptr;
    }
    if (size != 0)
    {
        m_TerritoriesLostSize = size;
        m_TerritoriesLostArray = new Territory[m_TerritoriesLostSize];
        for (uint32_t i = 0; i < size; i++)
            m_TerritoriesLostArray[i] = territories[i];

        if (m_TerritoriesLostSize == 1 && m_TerritoriesLostArray[0].name.length() > m_maxLetters)
        {
            int words = 0;

            String m_tempTerrName = "";
            int m_currentLineLength = 0;
            String m_word;
            for (int i = 0; i < m_TerritoriesLostArray[0].name.length(); i++)
            {
                if (m_TerritoriesLostArray[0].name.charAt(i) == ' ' || m_TerritoriesLostArray[0].name.charAt(i) == '\n') // Desert East Lower counted as 2 words
                {
                    if (m_currentLineLength + m_word.length() > m_maxLetters)
                    {
                        m_tempTerrName += "\n";
                        m_currentLineLength = 0;
                    }
                    words++;
                    m_tempTerrName += m_word + " ";
                    m_currentLineLength += m_word.length() + 1;
                    DEBUGf("Word %d: %s\n", words, m_word.c_str());
                    m_word = "";
                }
                else
                    m_word += m_TerritoriesLostArray[0].name.charAt(i);
            }

            if (m_currentLineLength + m_word.length() > m_maxLetters)
                m_tempTerrName += "\n";
            m_tempTerrName += m_word;
            m_TerritoriesLostArray[0].name = m_tempTerrName;
            DEBUGf("Words in Territory name: %d\n", words);
        }
        m_ForceUpdate = true;
        BuzzTheBuzzer(3, 50, 200);
    }
}

void IRAM_ATTR UiManager::ForceEnableDisplay()
{
    if (m_DisplayOn)
    {
        m_ScreenOnTime = millis();
        return;
    }
    m_DisplayOn = true;
    m_gfx->printf("Guild: ");
    m_gfx->setTextColor(DARKGREY);
    m_gfx->printf("%s\n", GUILD_PREFIX);
    m_gfx->setTextColor(WHITE);
    m_gfx->printf("Terrs: ");
    m_gfx->setTextColor(DARKGREY);
    m_gfx->printf("%d\n", m_TerritoriesTrackedSize);
    m_ScreenOnTime = millis();
    
}

void UiManager::ResetDisplay()
{
    // m_gfx->setTextSize(1);
    m_gfx->setCursor(0, 0);
    m_gfx->setTextColor(WHITE);
    m_gfx->fillScreen(BLACK);
    m_ForceUpdate = false;
    m_DisplayOn = false;
}

void UiManager::BuzzTheBuzzer(uint32_t times, uint32_t durationON, uint32_t durationOFF)
{
    for (uint32_t i = 0; i < times; i++)
    {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(durationON);
        digitalWrite(BUZZER_PIN, LOW);
        delay(durationOFF);
    }
}