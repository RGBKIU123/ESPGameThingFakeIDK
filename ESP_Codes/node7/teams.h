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
  { "TEAM01", "pass01", "FLAG{WORD_MASTER_XKW}", "GOAL", "projectcell", "room 301" },
  { "TEAM02", "pass02", "FLAG{WORD_MASTER_QMP}", "lightning", "projectcell", "Come to the floor where pcell works There I wear green but never grow, I speak to all though I have no voice. I wait where news becomes noise." },
  { "TEAM03", "pass03", "FLAG{WORD_MASTER_RLZ}", "lightning", "projectcell", "Come to the floor where pcell works There I wear green but never grow, I speak to all though I have no voice. I wait where news becomes noise." },
  { "TEAM04", "pass04", "FLAG{WORD_MASTER_VHY}", "lightning", "projectcell", "Come to the floor where pcell works There I wear green but never grow, I speak to all though I have no voice. I wait where news becomes noise." },
  { "TEAM05", "pass05", "FLAG{WORD_MASTER_DFG}", "lightning", "projectcell", "Come to the floor where pcell works There I wear green but never grow, I speak to all though I have no voice. I wait where news becomes noise." },
  { "TEAM06", "pass06", "FLAG{WORD_MASTER_UIO}", "lightning", "projectcell", "Come to the floor where pcell works There I wear green but never grow, I speak to all though I have no voice. I wait where news becomes noise." },
  { "TEAM07", "pass07", "FLAG{WORD_MASTER_ERT}", "lightning", "projectcell", "Come to the floor where pcell works There I wear green but never grow, I speak to all though I have no voice. I wait where news becomes noise." }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
