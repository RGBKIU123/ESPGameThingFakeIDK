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
  { "TEAM01", "pass01", "FLAG{M3M0RY_M4TR1X_M4ST3R_KXZ}", "fire", "projectcell", "I'm the path that climbs but doesn't roam, I wait where corridors stop calling home. When footsteps fade and halls grow bare, Your clue awaits — the last stairs there." },
  { "TEAM02", "pass02", "FLAG{M3M0RY_M4TR1X_M4ST3R_PLW}", "fire", "projectcell", "I'm the path that climbs but doesn't roam, I wait where corridors stop calling home. When footsteps fade and halls grow bare, Your clue awaits — the last stairs there." },
  { "TEAM03", "pass03", "FLAG{M3M0RY_M4TR1X_M4ST3R_QHM}", "fire", "projectcell", "I'm the path that climbs but doesn't roam, I wait where corridors stop calling home. When footsteps fade and halls grow bare, Your clue awaits — the last stairs there." },
  { "TEAM11", "pass11", "FLAG{M3M0RY_M4TR1X_M4ST3R_VBR}", "GOAL", "projectcell", "room 301" },
  { "TEAM12", "pass12", "FLAG{M3M0RY_M4TR1X_M4ST3R_ZYF}", "fire", "projectcell", "I'm the path that climbs but doesn't roam, I wait where corridors stop calling home. When footsteps fade and halls grow bare, Your clue awaits — the last stairs there." },
  { "TEAM13", "pass13", "FLAG{M3M0RY_M4TR1X_M4ST3R_NGJ}", "fire", "projectcell", "I'm the path that climbs but doesn't roam, I wait where corridors stop calling home. When footsteps fade and halls grow bare, Your clue awaits — the last stairs there." },
  { "TEAM14", "pass14", "FLAG{M3M0RY_M4TR1X_M4ST3R_LFB}", "fire", "projectcell", "I'm the path that climbs but doesn't roam, I wait where corridors stop calling home. When footsteps fade and halls grow bare, Your clue awaits — the last stairs there." }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
