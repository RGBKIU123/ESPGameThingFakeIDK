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
  { "TEAM03", "pass03", "FLAG{STACK_MASTER_KWX}", "GOAL", "projectcell", "room 301" },
  { "TEAM04", "pass04", "FLAG{STACK_MASTER_PMQ}", "light", "projectcell", "not mid, not hoop, not player's lane, But where the edges frame the game." },
  { "TEAM05", "pass05", "FLAG{STACK_MASTER_RLZ}", "light", "projectcell", "not mid, not hoop, not player's lane, But where the edges frame the game." },
  { "TEAM06", "pass06", "FLAG{STACK_MASTER_VHY}", "light", "projectcell", "not mid, not hoop, not player's lane, But where the edges frame the game." },
  { "TEAM07", "pass07", "FLAG{STACK_MASTER_BFN}", "light", "projectcell", "not mid, not hoop, not player's lane, But where the edges frame the game." },
  { "TEAM08", "pass08", "FLAG{STACK_MASTER_GJM}", "light", "projectcell", "not mid, not hoop, not player's lane, But where the edges frame the game." },
  { "TEAM09", "pass09", "FLAG{STACK_MASTER_TKP}", "light", "projectcell", "not mid, not hoop, not player's lane, But where the edges frame the game." }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
