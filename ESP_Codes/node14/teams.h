#ifndef TEAMS_H
#define TEAMS_H

struct TeamData {
  const char* teamId;
  const char* password;
  const char* flag;
  const char* nextWifi;
  const char* nextWifiPassword;
  const char* clue;
};

// Define team data - generated from flag_list.txt
const TeamData teams[] = {
  { "TEAM08", "pass08", "FLAG{ULTIMATE_CHAMPION_KWX}", "GOAL", "projectcell", "room 301" },
  { "TEAM09", "pass09", "FLAG{ULTIMATE_CHAMPION_PMQ}", "air", "projectcell", "I wear red though I never bleed, I hush the flames and do the deed. Not ground level, not sky high — just the first step to the view." },
  { "TEAM10", "pass10", "FLAG{ULTIMATE_CHAMPION_RLZ}", "air", "projectcell", "I wear red though I never bleed, I hush the flames and do the deed. Not ground level, not sky high — just the first step to the view." },
  { "TEAM11", "pass11", "FLAG{ULTIMATE_CHAMPION_VHY}", "air", "projectcell", "I wear red though I never bleed, I hush the flames and do the deed. Not ground level, not sky high — just the first step to the view." },
  { "TEAM12", "pass12", "FLAG{ULTIMATE_CHAMPION_BFN}", "air", "projectcell", "I wear red though I never bleed, I hush the flames and do the deed. Not ground level, not sky high — just the first step to the view." },
  { "TEAM13", "pass13", "FLAG{ULTIMATE_CHAMPION_GJM}", "air", "projectcell", "I wear red though I never bleed, I hush the flames and do the deed. Not ground level, not sky high — just the first step to the view." },
  { "TEAM14", "pass14", "FLAG{ULTIMATE_CHAMPION_WDV}", "air", "projectcell", "I wear red though I never bleed, I hush the flames and do the deed. Not ground level, not sky high — just the first step to the view." },
  { "TEAM15", "pass15", "FLAG{ULTIMATE_CHAMPION_VHK}", "fire", "projectcell", "I’m the path that climbs but doesn’t roam,I wait where corridors stop calling home.When footsteps fade and halls grow bare,Your clue awaits — the last stairs there" }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
