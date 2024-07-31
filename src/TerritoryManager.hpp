#pragma once
#include "Territory.hpp"
#include "GlobalDebugSetup.h"

#define MAX_TERRITORIES 406

class TerritoryManager
{
public:
    TerritoryManager(){};
    ~TerritoryManager(){};

    Territory *GetLiveTerritoriesPtr() { return liveTerritories; }
    void UpdateTrackedTerrList(Stream &, const char *);
    uint32_t GetTracedTerrCount() { return territoriesSize; }
    Territory *GetTrackedTerrLost(uint32_t &size)
    {
        size = territoriesLostSize;
        return territoriesLostArray;
    }

private:
    uint32_t territoriesSize = 0;
    Territory *territoriesArray = new Territory[territoriesSize];
    uint32_t territoriesLostSize = 0;
    Territory *territoriesLostArray = new Territory[territoriesLostSize];
    Territory liveTerritories[MAX_TERRITORIES];
} TerritoryManager;

void TerritoryManager::UpdateTrackedTerrList(Stream &payload, const char *guildPrefix)
{
    territoriesLostSize = 0;
    delete[] territoriesLostArray;
    territoriesLostArray = nullptr;

    // Parse the JSON
    uint32_t liveTerritoriesCount = 0;
    payload.find("{");
    while (payload.available())
    {
        if (payload.find("\""))
        {
            String m_tempString = payload.readStringUntil('"');
            liveTerritories[liveTerritoriesCount].name = m_tempString;

            payload.find("\"prefix\":\"");
            m_tempString = payload.readStringUntil('"');
            m_tempString.toCharArray(liveTerritories[liveTerritoriesCount].guildPrefix, 5);

            liveTerritoriesCount++;
        }
        payload.find("}},");
    }

    // If this is True, something went wrong in the parsing so don't continue
    if (liveTerritoriesCount != MAX_TERRITORIES)
        return;

    // Count the new number of tracked territories
    uint32_t m_countedTerrs = 0;
    for (uint32_t i = 0; i < MAX_TERRITORIES; i++)
        if (liveTerritories[i].guildPrefix[0] == guildPrefix[0] && liveTerritories[i].guildPrefix[1] == guildPrefix[1] && liveTerritories[i].guildPrefix[2] == guildPrefix[2] && liveTerritories[i].guildPrefix[3] == guildPrefix[3])
            m_countedTerrs++;

    // Create a new list of tracked territories, updated to live data
    uint32_t m_tempCountedTerrs = 0;
    Territory *m_tempTrackedTerrs = new Territory[m_countedTerrs];
    for (uint32_t i = 0; i < MAX_TERRITORIES; i++)
    {
        if (liveTerritories[i].guildPrefix[0] == guildPrefix[0] && liveTerritories[i].guildPrefix[1] == guildPrefix[1] && liveTerritories[i].guildPrefix[2] == guildPrefix[2] && liveTerritories[i].guildPrefix[3] == guildPrefix[3])
        {
            m_tempTrackedTerrs[m_tempCountedTerrs].name = liveTerritories[i].name;
            m_tempTrackedTerrs[m_tempCountedTerrs].guildPrefix[0] = liveTerritories[i].guildPrefix[0];
            m_tempTrackedTerrs[m_tempCountedTerrs].guildPrefix[1] = liveTerritories[i].guildPrefix[1];
            m_tempTrackedTerrs[m_tempCountedTerrs].guildPrefix[2] = liveTerritories[i].guildPrefix[2];
            m_tempTrackedTerrs[m_tempCountedTerrs].guildPrefix[3] = liveTerritories[i].guildPrefix[3];
            m_tempTrackedTerrs[m_tempCountedTerrs].guildPrefix[4] = '\0';

            // DEBUGt(F("Territory: "));
            // DEBUGt(m_tempTrackedTerrs[m_tempCountedTerrs].name);
            // DEBUGt(F("    GuildPrefix: "));
            // DEBUGln(m_tempTrackedTerrs[m_tempCountedTerrs].guildPrefix);

            m_tempCountedTerrs++;
        }
    }

    // Find out what terrs we lost
    for (uint32_t i = 0; i < territoriesSize; i++) // This no worke rn
    {
        Territory m_tempTerr = territoriesArray[i];
        bool found = false;
        uint32_t j = 0;
        while (j < m_countedTerrs && !found)
        {
            Territory m_tempTerr2 = m_tempTrackedTerrs[j];
            if (m_tempTerr.name == m_tempTerr2.name && m_tempTerr.guildPrefix[0] == m_tempTerr2.guildPrefix[0] && m_tempTerr.guildPrefix[1] == m_tempTerr2.guildPrefix[1] && m_tempTerr.guildPrefix[2] == m_tempTerr2.guildPrefix[2] && m_tempTerr.guildPrefix[3] == m_tempTerr2.guildPrefix[3])
                found = true;
            j++;
        }
        if (!found)
        {
            Territory *m_tempTerrLostArray = new Territory[territoriesLostSize + 1];

            /*
            if (territoriesLostSize != 0)
                memccpy(m_tempTerrLostArray, territoriesLostArray, 0, sizeof(Territory) * territoriesLostSize);
            */

            for (uint32_t i = 0; i < territoriesLostSize; i++)
                m_tempTerrLostArray[i] = territoriesLostArray[i];

            m_tempTerrLostArray[territoriesLostSize] = m_tempTerr;
            delete[] territoriesLostArray;
            territoriesLostArray = m_tempTerrLostArray;
            territoriesLostSize++;
        }
    }

    // Figure out to who each terr was lost to
    for (uint32_t i = 0; i < territoriesLostSize; i++)
    {
        Territory m_tempTerr = territoriesLostArray[i];
        bool found = false;
        uint32_t j = 0;
        while (j < MAX_TERRITORIES && !found)
        {
            Territory m_tempTerr2 = liveTerritories[j];
            if (m_tempTerr.name == m_tempTerr2.name)
                found = true;
            j++;
        }
        if (found)
            territoriesLostArray[i] = liveTerritories[j - 1];
        else
        {
            territoriesLostArray[i].guildPrefix[0] = '?';
            territoriesLostArray[i].guildPrefix[1] = '?';
            territoriesLostArray[i].guildPrefix[2] = '?';
            territoriesLostArray[i].guildPrefix[3] = '?';
            territoriesLostArray[i].guildPrefix[0] = '\0';
        }
    }

    delete[] territoriesArray;
    territoriesSize = m_countedTerrs;
    territoriesArray = m_tempTrackedTerrs;
}
